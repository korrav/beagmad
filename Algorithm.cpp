/*
 * Algorithm.cpp
 *
 *  Created on: 01 февр. 2014 г.
 *      Author: andrej
 */

#include "Algorithm.h"
#include "Mad.h"
#include <iostream>

namespace mad_n {

Algorithm::Algorithm(std::string name, const int& id, void (*pf)(std::vector<int8_t>&, int)) :
				name_(name), id_(id), pass__(pf) {
	return;
}

void Algorithm::close_(void) {
	if (isRunThread_) {
		isRunThread_ = false;
		isEmptyAndClose_.notify_one();
		end_.get();
	}
	return;
}

Algorithm::~Algorithm() {
	close_();
	return;
}

void Algorithm::push_fifo(std::shared_ptr<const DataADC> d) {
	mut_fifo_.lock();
	fifo_.push_back(d);
	isEmptyAndClose_.notify_one();
	mut_fifo_.unlock();
	return;
}

void Algorithm::clear_fifo(void) {
	mut_fifo_.lock();
	fifo_.clear();
	mut_fifo_.unlock();
	return;
}

std::shared_ptr<const DataADC> Algorithm::pop_fifo(void) {
	std::shared_ptr<const DataADC> data;
	std::unique_lock<std::mutex> lk(mut_fifo_);
	isEmptyAndClose_.wait(lk,
			[this] {return (!fifo_.empty() || !isRunThread_);});
	if (isRunThread_) {
		data = std::move(fifo_.front());
		fifo_.pop_front();
		lk.unlock();
		wdtIn_.write(data->get_data(), data->get_amount() * 4 * sizeof(short));
		return data;
	} else
		return nullptr;
}

void Algorithm::set_task_in(const std::string& nameFile, const int& num) {
	if (isRunThread_) {
		bool status = wdtIn_.set_task(nameFile, num);
		std::cout << "Для алгоритма " << name_
				<< (status ? " создано " : " не возможно создать ")
				<< "задание на запись входных данных\n";
	} else
		std::cout << "Алгоритм " << name_ << " сейчас неактивен\n";
	return;
}

void Algorithm::set_task_out(const std::string& nameFile, const int& num) {
	if (isRunThread_) {
		bool status = wdtOut_.set_task(nameFile, num);
		std::cout << "Для алгоритма " << name_
				<< (status ? " создано " : " не возможно создать ")
				<< "задание на запись выходных данных\n";
	} else
		std::cout << "Алгоритм " << name_ << " сейчас неактивен\n";
	return;
}

int Algorithm::get_count_queue(void) {
	int num = 0;
	mut_fifo_.lock();
	num = fifo_.size();
	mut_fifo_.unlock();
	return num;
}

void Algorithm::pass_(void* pbuf, size_t size, int id_block, std::shared_ptr<const DataADC> pDataBuf) {
	wdtOut_.write(pbuf, size);
	Head config = Mad::getConfig();
	pDataBuf->get_gain(config.gain);
	config.freq = pDataBuf->get_freq();
	config.numAlg = id_block;
	size_t sizeFull = sizeof(Head) + size;
	std::vector<int8_t> pBufFull(sizeFull);
	auto iterator = std::copy_n(reinterpret_cast<int8_t*>(&config), sizeof(config), pBufFull.begin());
	std::copy_n(reinterpret_cast<int8_t*>(pbuf), size, iterator);
	pass__(pBufFull, DATA);
	return;
}

} /* namespace mad_n */
