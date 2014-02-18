/*
 * ManagerAlg.cpp
 *
 *  Created on: 01 февр. 2014 г.
 *      Author: andrej
 */

#include "ManagerAlg.h"
#include <iostream>

namespace mad_n {

ManagerAlg::ManagerAlg(SinkAdcData* s) :
		supplier_(s) {
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
			<< "установлен агоритм " << a->get_name() << std::endl;
	return status;
}

bool ManagerAlg::turnOn(const std::string& alg) {
	if (setA_.count(alg)) {
		setA_[alg]->open_();
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

ManagerAlg::~ManagerAlg() {
	closeDistributor();
	return;
}

} /* namespace mad_n */

