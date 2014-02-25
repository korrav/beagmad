/*
 * SinkAdcData.cpp
 *
 *  Created on: 01 февр. 2014 г.
 *      Author: andrej
 */

#include "SinkAdcData.h"
#include <sys/stat.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <iostream>

namespace mad_n {

SinkAdcData::SinkAdcData(const char* dev) :
		isRunThread_(false), freq_(0) {
	f_adc_ = open(dev, O_RDWR);
	if (f_adc_ == -1) {
		std::cerr << "Невозможно открыть файл устройства АЦП\n";
		exit(1);
	}
	openMain();
	return;
}

void SinkAdcData::openMain(void) {
	isRunThread_ = true;
	end_ = std::async(std::launch::async, &SinkAdcData::main, this);
	return;
}

void SinkAdcData::closeMain(void) {
	if (isRunThread_) {
		isRunThread_ = false;
		end_.get();
	}
	return;
}

void SinkAdcData::main(void) {
	DataADC* d = nullptr;
	for (;;) {
		if (!isRunThread_)
			break;
		if (d == nullptr)
			d = new DataADC;
		if (ioctl(f_adc_, IOCTL_ADC_GET_MESSAGE, d->get_to_adc_driver()) < 0) {
			std::cout << "Приём данных от f3 окончился неудачей\n";
			continue;
		}
		if (d->get_amount() == 0) {
			d->set_amount_init();
			continue;
		}
		d->set_gain(gain_);
		d->set_freq(freq_);
		wdtIn_.write(d->get_data(), d->get_amount() * 4 * sizeof(short));
		mut_.lock();
		fifo_.push_back(std::unique_ptr<DataADC>(d));
		isEmpty_.notify_one();
		mut_.unlock();
		d = nullptr;
	}
	mut_.lock();
	fifo_.clear();
	isEmpty_.notify_one();
	mut_.unlock();
	return;
}

std::shared_ptr<DataADC> SinkAdcData::obtainData(void) {
	std::shared_ptr<DataADC> data;
	std::unique_lock<std::mutex> lk(mut_);
	isEmpty_.wait(lk, [this] {return (!fifo_.empty() || !isRunThread_);});
	if (isRunThread_) {
		data = std::move(fifo_.front());
		fifo_.pop_front();
		return data;
	} else
		return nullptr;
}

void SinkAdcData::set_freq(const int& f) {
	freq_ = f;
	return;
}

int SinkAdcData::get_freq(void) {
	return freq_;
}

void SinkAdcData::set_task_in(const std::string& nameFile, const int& num) {
	if (isRunThread_) {
		bool status = wdtIn_.set_task(nameFile, num);
		std::cout << "Для потока приёмника данных от АЦП"
				<< (status ? " создано " : " не возможно создать ")
				<< "задание на запись входных данных\n";
	} else
		std::cout << "Поток приёмника данных от АЦП " << " сейчас неактивен\n";
	return;
}

int SinkAdcData::get_count_queue(void) {
	int num = 0;
	mut_.lock();
	num = fifo_.size();
	mut_.unlock();
	return num;
}

SinkAdcData::~SinkAdcData() {
	closeMain();
	close(f_adc_);
	return;
}

void SinkAdcData::set_gain(short * g) {
	for (short& x : gain_)
		x = *(g++);
	return;
}

void SinkAdcData::get_gain(short * g) {
	for (short& x : gain_)
		*(g++) = x;
	return;
}

} /* namespace mad_n */
