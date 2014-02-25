/*
 * DataADC.cpp
 *
 *  Created on: 01 февр. 2014 г.
 *      Author: andrej
 */

#include "DataADC.h"

namespace mad_n {

DataADC::DataADC() {
	b_adc_.pUnit = new short[BUF_SIZE / sizeof(short)];
	b_adc_.amountCount = BUF_SIZE / (sizeof(short) * 4);
	b_adc_.error.error = 0;
	freq_ = 0;
	return;
}

DataADC& DataADC::operator =(const DataADC& data) {
	if (this == &data)
		return *this;
	for(int i = 0; i < 4; i++)
		gain_[i] = data.gain_[i];
	freq_ = data.freq_;
	b_adc_.error = data.b_adc_.error;
	b_adc_.mode = data.b_adc_.mode;
	b_adc_.amountCount = data.b_adc_.amountCount;
	b_adc_.count = data.b_adc_.count;
	delete[] b_adc_.pUnit;
	b_adc_.pUnit = new short[b_adc_.amountCount * 4];
	for (unsigned int i = 0; i < 4 * b_adc_.amountCount; i++)
		b_adc_.pUnit[i] = data.b_adc_.pUnit[i];
	return *this;
}

DataADC::~DataADC() {
	delete[] b_adc_.pUnit;
	return;
}

} /* namespace mad_n */
