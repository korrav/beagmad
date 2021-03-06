/*
 * ExcessNoiseAlg.cpp
 *
 *  Created on: 18 февр. 2014 г.
 *      Author: andrej
 */

#include "ExcessNoiseAlg.h"
#include <iostream>
#include "ManagerAlg.h"
#include <string.h>
#include <algorithm>
#include <fstream>
#include <cstddef>
namespace mad_n {

std::shared_ptr<const DataADC> ExcessNoiseAlg::pd_ = nullptr;

ExcessNoiseAlg::ExcessNoiseAlg(std::string name, const int& id,
		void (*pf)(std::vector<int8_t>&, int), ManagerAlg* man) :
								Algorithm(name, id, pf), numFirstCount_(0), beforeEvent_(
										INIT_BEFORE_EVENT), afterEvent_(INIT_AFTER_EVENT), sigma_(
												INIT_SIGMA), man_(man), isEnableWriteFullBlock_(false) {
	pBuf_ = new int8_t[sizeof(Gasik) + (MAX_BEFORE_EVENT + MAX_AFTER_EVENT) * sizeof(short)];
}

bool ExcessNoiseAlg::open_(void) {
	if (check_valid_theard()) {
		std::cout << "Алгоритм " << get_name() << " уже запущен\n";
		return false;
	}
	Algorithm::open__();
	end_ = std::async(std::launch::async, &ExcessNoiseAlg::excessNoise, this);
	return true;
}

void ExcessNoiseAlg::excessNoise(void) {
	struct index;
	std::vector<short>::iterator pcur;
	int rms[4];
	int mean[4];
	std::vector<short> samples; //приёмный буфер
	unsigned int beforeEvent, afterEvent;
	try {
		for (;;) {
			if (!check_valid_theard())
				break;
			//получаю текущее состояние статистики
			man_->get_rms(rms);
			man_->get_mean(mean);
			//получаю размер пакета, содержащего событие а также положение событя в нём
			get_parameter_block(&beforeEvent, &afterEvent);
			increase_size_buf(samples, (beforeEvent + afterEvent) * 4);
			pcur = samples.begin() + 4 * beforeEvent;	//указатель на положение события

			//проверка на наличие отсчётов, превышающих уровень мощности
			searchIncreaseNoise(pcur, samples.end(), rms, mean);
			if (pcur < samples.end()) {	//событие найдено
				int idx_cur = pcur - samples.begin();	//индекс положения cj,znbz
				if (pcur + 4 * afterEvent < samples.end())	//если событие полностью помещается в текущий буфер
					transfer_data(samples,
							samples.begin() + idx_cur - beforeEvent * 4,
							4 * (beforeEvent + afterEvent));
				else {										//если событие не полностью помещается в текущий буфер
					increase_size_buf(samples,
							samples.size() + 4 * afterEvent
							- (samples.end() - pcur));
					transfer_data(samples,
							samples.begin() + idx_cur - 4 * beforeEvent,
							4 * (beforeEvent + afterEvent));
				}
				auto end = samples.begin() + idx_cur + 4 * afterEvent;	//конечная позиция события
				//обрезка начала буфера и соответственное обновление номера первого отсчёта буфера
				samples.erase(samples.begin(), end);
				numFirstCount_ += (end - samples.begin()) / 4;
			} else {		//событие не найдено
				//обрезка начала буфера и соответственное обновление номера первого отсчёта буфера
				auto end = samples.end() - 4 * beforeEvent;
				samples.erase(samples.begin(), end);
				numFirstCount_ += (end - samples.begin()) / 4;
			}

		}
	} catch (excep_out_of_algorithm&) {

	}
	samples.clear();
	clear_fifo();
	return;
}

void ExcessNoiseAlg::increase_size_buf(std::vector<short>& buf,
		unsigned int size) {
	excep_out_of_algorithm out;
	pd_ = nullptr;
	while (buf.size() < size) {
		pd_ = pop_fifo();
		if (pd_ != nullptr) {
			buf.insert(buf.end(), pd_->get_buf(),
					pd_->get_buf() + pd_->get_amount() * 4);
		}
		if (!check_valid_theard())
			throw out;
	}
	if (pd_ != nullptr)
		numFirstCount_ = pd_->get_first() - (buf.size() / 4 - pd_->get_amount());
	return;

}

void ExcessNoiseAlg::transfer_data(std::vector<short>& v,
		const std::vector<short>::iterator& cur, const unsigned &num_sampl) {
	std::vector<short>::iterator pt = cur, end = cur + num_sampl;
	pBuf_->buf.numFirstCount = (cur - v.begin()) + numFirstCount_;
	pBuf_->param.level = sigma_;

	std::copy_n(cur, num_sampl, reinterpret_cast<short*>(&pBuf_->buf.data));
	size_t sizeBuf = num_sampl * sizeof(short) + offsetof(Gasik, buf) + offsetof(DataAlgorithm, data);
	if(pd_ != nullptr)
		pass_(pBuf_, sizeBuf, GASIK, pd_);
	return;

}

void ExcessNoiseAlg::writeFullBlock(void) {
	if (!isEnableWriteFullBlock_)
		return;
	char* pbuf;
	if (pd_ == nullptr)
		return;
	else
		pbuf = reinterpret_cast<char*>(pd_->get_data());
	std::ofstream file(NameFileFullBlock_,
			std::ios::out | std::ios::binary | std::ios::trunc);
	if (!file.is_open()) {
		std::cout << "Нет возможности создать файл " << NameFileFullBlock_
				<< std::endl;
		return;
	} else {
		file.write(pbuf, pd_->get_amount() * 4 * sizeof(short));
		std::cout << "В файл " << NameFileFullBlock_ << " записано "
				<< pd_->get_amount() << " отсчётов\n";
		isEnableWriteFullBlock_ = false;
	}
	return;
}

ExcessNoiseAlg::~ExcessNoiseAlg() {
	delete[] reinterpret_cast<int8_t*>(pBuf_);
}

void ExcessNoiseAlg::searchIncreaseNoise(std::vector<short>::iterator& cur,
		const std::vector<short>::iterator& end, int* rms, int* mean) {
	for (; cur < end; cur += 4) { //интерация по отсчётам
		for (int ch = 0; ch < 4; ch++) { //интерация по каналам
			if ((*(cur + ch) >= mean[ch] + sigma_ * rms[ch])
					|| (*(cur + ch) <= mean[ch] - sigma_ * rms[ch])) {
				writeFullBlock();
				return;
			}
		}
	}
}

}
/* namespace mad_n */
