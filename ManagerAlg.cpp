/*
 * ManagerAlg.cpp
 *
 *  Created on: 01 февр. 2014 г.
 *      Author: andrej
 */

#include "ManagerAlg.h"
#include <iostream>

namespace mad_n {

ManagerAlg::ManagerAlg(SinkAdcData* s, void (*pf)(void*, size_t, int)) :
		supplier_(s), monitor_(pf) {
	if (supplier_ == nullptr) {
		std::cout << "Поставщик буферов данных не инициализирован\n";
		exit(1);
	}
	openDistributor();
	return;
}

void ManagerAlg::openDistributor(void) {
	isRunThread_ = true;
	end_ = std::async(std::launch::async, &ManagerAlg::distributor, this);
	return;
}

void ManagerAlg::closeDistributor(void) {
	if (isRunThread_) {
		turnOffAll();
		isRunThread_ = false;
		end_.get();
	}
	return;
}

bool ManagerAlg::addAlgorithm(Algorithm* a) {
	bool status = setA_.insert(std::make_pair(a->get_name(), a)).second;
	std::cout << "В менеджере алгоритмов " << (status ? "" : "не ")
			<< "установлен алгоритм " << a->get_name() << std::endl;
	return status;
}

bool ManagerAlg::turnOn(const std::string& alg) {
	if (setA_.count(alg)) {
		if (setA_[alg]->open_())
			std::cout << "Включен алгоритм " << alg << std::endl;
		return true;
	} else {
		std::cout << "Алгоритма " << alg << " нет в пуле менеджера алгоритмов"
				<< std::endl;
		return false;
	}
}

bool ManagerAlg::turnOn(const int& alg) {
	for (auto& p : setA_) {
		if (p.second->get_id() == alg) {
			p.second->open_();
			return true;
		}
	}
	return false;
}

bool ManagerAlg::turnOff(const std::string& alg) {
	if (setA_.count(alg)) {
		setA_[alg]->close_();
		std::cout << "Выключен алгоритм " << alg << std::endl;
		return true;
	} else {
		std::cout << "Алгоритм " << alg << " нет в пуле менеджера алгоритмов"
				<< std::endl;
		return false;
	}
}
bool ManagerAlg::turnOff(const int& alg) {
	for (auto& p : setA_) {
		if (p.second->get_id() == alg) {
			p.second->close_();
			return true;
		}
	}
	return false;
}

void ManagerAlg::distributor(void) {
	std::shared_ptr<const DataADC> pd;
	for (;;) {
		if (!isRunThread_)
			break;
		pd = supplier_->obtainData();
		if (pd == nullptr)
			continue;
		monitor_.calculation_stats(*pd);
		for (auto palg = setA_.begin(); palg != setA_.end(); palg++) {
			if (palg->second->check_valid_therad())
				palg->second->push_fifo(pd);
		}

	}
	return;
}

void ManagerAlg::turnOffAll(void) {
	for (auto & alg : setA_)
		alg.second->close_();
	return;
}

void ManagerAlg::set_task_in(const std::string nameAlg,
		const std::string& nameFile, const int& num) {
	if (setA_.count(nameAlg))
		setA_[nameAlg]->set_task_in(nameFile, num);
	else
		std::cout << "Алгоритма " << nameAlg << " не существует\n";
	return;
}

void ManagerAlg::set_task_out(const std::string nameAlg,
		const std::string& nameFile, const int& num) {
	if (setA_.count(nameAlg))
		setA_[nameAlg]->set_task_out(nameFile, num);
	else
		std::cout << "Алгоритма " << nameAlg << " не существует\n";
	return;
}

int ManagerAlg::get_count_queue(const std::string& name) {
	int num = 0;
	if (setA_.count(name))
		num = setA_[name]->get_count_queue();
	else {
		std::cout << "Алгоритма " << name << " не существует\n";
		return -1;
	}
	return num;
}

void ManagerAlg::get_rms(int *prms) {
	monitor_.get_statistics(prms, nullptr);
	return;
}

void ManagerAlg::get_mean(int* pmean) {
	monitor_.get_statistics(nullptr, pmean);
	return;
}

ManagerAlg::~ManagerAlg() {
	closeDistributor();
	return;
}

void Monitor::set_statistics(const int* rms, const int* mean) {
	std::lock_guard<std::mutex> lk(mut_);
	if (mean)
		for (int i = 0; i < 4; i++)
			mean_[i] = mean[i];
	if (rms)
		for (int i = 0; i < 4; i++)
			rms_[i] = rms[i];
	return;
}

void Monitor::get_statistics(int* rms, int* mean) const {
	std::lock_guard<std::mutex> lk(mut_);
	if (rms != nullptr) {
		for (int i = 0; i < 4; i++)
			rms[i] = rms_[i];
	}
	if (mean != nullptr) {
		for (int i = 0; i < 4; i++)
			mean[i] = mean_[i];
	}
	return;
}

void Monitor::calculation_stats(const DataADC& bData) {
	const short *pbuf = reinterpret_cast<short *>(bData.get_data());
	static int num_sampl_monitorogram = 0; // количество отсчётов, уже учтённых при при вычисление текущей статистики
	static double sum[4] = { 0, 0, 0, 0 }; //сумма выборок
	static long double sumsquares[4] = { 0, 0, 0, 0 }; //сумма квадратов выборок
	//вычисление суммы значений отсчётов и суммы квадратов значений отсчётов
	unsigned int amount = bData.get_amount();
	for (unsigned int i = 0; i < amount; i = i + 4) { //интерации по отсчётам
		for (int j = 0; j < 4; j++) { //интерации по каналам
			sum[j] += pbuf[4 * i + j];
			sumsquares[j] += pow(static_cast<double>(pbuf[4 * i + j]), 2);
		}
		num_sampl_monitorogram++;
		//вычисление мат. ожидания и СКО
		if (num_sampl_monitorogram > AMOUNT_AN) {
			int mean[4];
			int rms[4];
			for (int k = 0; k < 4; k++) {
				mean[k] = sum[k] / AMOUNT_AN;
				rms[k] = sqrt(
						(sumsquares[k] - pow(sum[k], 2) / AMOUNT_AN)
								/ (AMOUNT_AN - 1));
			}
			for (int k = 0; k < 4; k++) {
				sum[k] = 0;
				sumsquares[k] = 0;
			}
			set_statistics(rms, mean);
			num_sampl_monitorogram = 0;
		}
	}
	if (std::chrono::steady_clock::now() >= transferTime_) {
		transferTime_ += periodTransfer_;
		bufM_.freq = bData.get_freq();
		bData.get_gain(bufM_.gain);
		get_statistics(bufM_.rms, bufM_.mean);
		pass_(&bufM_, sizeof(bufM_), MONITOR);
	}
	return;
}

Monitor::Monitor(void (*pf)(void*, size_t, int)) :
		pass_(pf) {
	transferTime_ = std::chrono::steady_clock::now() + periodTransfer_;
	return;
}

void ManagerAlg::get_active_algorithm(std::list<std::string>* pls,
		std::list<int>* pli) {
	if (pls != nullptr)
		for (auto& a : setA_) {
			if (a.second->check_valid_therad()) {
				pls->push_back(a.first);
			}
		}
	if (pli != nullptr)
		for (auto& a : setA_) {
			if (a.second->check_valid_therad()) {
				pli->push_back(a.second->get_id());
			}
		}
	return;
}

} /* namespace mad_n */
