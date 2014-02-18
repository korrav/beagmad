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

DataADC::~DataADC() {
	delete[] b_adc_.pUnit;
	return;
}

} /* namespace mad_n */
