/*
 * Algorithm.cpp
 *
 *  Created on: 01 февр. 2014 г.
 *      Author: andrej
 */

#include "Algorithm.h"

namespace mad_n {

Algorithm::Algorithm(std::string name, const int& id,
		void (*pf)(void*, size_t, int)) :
		name_(name), id_(id), pass_(pf) {
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
		return data;
	} else
		return nullptr;
}

} /* namespace mad_n */
