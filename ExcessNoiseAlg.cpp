/*
 * ExcessNoiseAlg.cpp
 *
 *  Created on: 18 февр. 2014 г.
 *      Author: andrej
 */

#include "ExcessNoiseAlg.h"
#include <iostream>
#include "ManagerAlg.h"
#include<string.h>
#include <algorithm>
#include <fstream>
namespace mad_n {

std::shared_ptr<const DataADC> ExcessNoiseAlg::pd_ = nullptr;

ExcessNoiseAlg::ExcessNoiseAlg(std::string name, const int& id,
		void (*pf)(void*, size_t, int), ManagerAlg* man) :
		Algorithm(name, id, pf), numFirstCount_(0), beforeEvent_(
				INIT_BEFORE_EVENT), afterEvent_(INIT_AFTER_EVENT), sigma_(
				INIT_SIGMA), man_(man), isEnableWriteFullBlock_(false) {
}

bool ExcessNoiseAlg::open_(void) {
	if (check_valid_therad()) {
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
			if (!check_valid_therad())
				break;
			man_->get_rms(rms);
			man_->get_mean(mean);
			get_parameter_block(&beforeEvent, &afterEvent);
			increase_size_buf(samples, (beforeEvent + afterEvent) * 4);
			pcur = samples.begin() + 4 * beforeEvent;

			//проверка на наличие отсчётов, превышающих уровень мощности
			searchIncreaseNoise(pcur, samples.end(), rms, mean);
			if (pcur < samples.end()) {
				int idx_cur = pcur - samples.begin();
				if (pcur + 4 * afterEvent < samples.end())
					transfer_data(samples,
							samples.begin() + idx_cur - beforeEvent * 4,
							4 * (beforeEvent + afterEvent));
				else {
					increase_size_buf(samples,
							samples.size() + 4 * afterEvent
									- (samples.end() - pcur));
					transfer_data(samples,
							samples.begin() + idx_cur - 4 * beforeEvent,
							4 * (beforeEvent + afterEvent));
				}
				auto end = samples.begin() + idx_cur + 4 * afterEvent;
				samples.erase(samples.begin(), end);
				numFirstCount_ += (end - samples.begin()) / 4;
			} else {
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
	int index = 0;
	while (buf.size() < size) {
		pd_ = pop_fifo();
		if (pd_ != nullptr) {
			index = buf.size() / 4;
			buf.insert(buf.end(), pd_->get_buf(),
					pd_->get_buf() + pd_->get_amount() * 4);
		}
		if (!check_valid_therad())
			throw out;
	}
	if (pd_ != nullptr) {
		numFirstCount_ = (pd_->get_first() - index) / 4;
		buf_.freq = pd_->get_freq();
		pd_->get_gain(buf_.gain);
	}
	return;

}

void ExcessNoiseAlg::transfer_data(std::vector<short>& v,
		const std::vector<short>::iterator& cur, const unsigned &num_sampl) {
	std::copy(cur, cur + num_sampl, buf_.sampl);
	buf_.amountCount = num_sampl / 4;
	buf_.numFirstCount = (cur - v.begin()) + numFirstCount_;
	pass_(&buf_,
			sizeof(buf_) - (4 * NUM_SAMPL_PACK - num_sampl) * sizeof(short),
			FILTER_NOISE);
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
	std::cout << "cur " << (cur == end ? "equ" : "not equ") << " end\n";
	return;
}

}
/* namespace mad_n */
