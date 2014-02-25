/*
 * DataADC.h
 *
 *  Created on: 01 февр. 2014 г.
 *      Author: andrej
 */

#ifndef DATAADC_H_
#define DATAADC_H_

#include "f3_spi.h"

namespace mad_n {

/*
 * Объекты этого класса являются блоками данных, заполняемых АЦП
 */
class DataADC {
	dataUnit_ADC b_adc_;
	short gain_[4]; //коэффициенты усиления
	int freq_; //частота дискретизации(в Гц)
public:
	DataADC& operator =(const DataADC&);
	inline short* get_buf(void);
	inline unsigned int get_first(void);
	inline unsigned int get_amount(void) const;
	inline void set_amount(unsigned int amount);
	inline void set_amount_init(void);
	inline void* get_to_adc_driver(void);
	inline void* get_data(void) const;
	inline adc_mode get_mode(void);
	inline void set_gain(short* g);
	inline void get_gain(short* g) const;
	inline void set_freq(const int& freq);
	inline unsigned int get_freq(void) const;
	DataADC();
	virtual ~DataADC();
};

inline short *DataADC::get_buf(void) {
	return b_adc_.pUnit;
}

unsigned int DataADC::get_first(void) {
	return b_adc_.count;
}

unsigned int DataADC::get_amount(void) const {
	return b_adc_.amountCount;
}

void* DataADC::get_to_adc_driver(void) {
	return reinterpret_cast<void*>(&b_adc_);
}

inline void* DataADC::get_data(void) const {
	return reinterpret_cast<void*>(b_adc_.pUnit);
}

adc_mode DataADC::get_mode(void) {
	return b_adc_.mode;
}

void DataADC::set_amount(unsigned int amount) {
	if (amount > BUF_SIZE / sizeof(short))
		b_adc_.amountCount = BUF_SIZE / sizeof(short);
	else
		b_adc_.amountCount = amount;
	return;
}
void DataADC::set_amount_init(void) {
	b_adc_.amountCount = BUF_SIZE / sizeof(short);
	return;
}

void DataADC::set_gain(short* g) {
	for (short& x : gain_)
		x = *(g++);
	return;
}

inline void DataADC::get_gain(short * g) const {
	for (const short& x : gain_)
		*(g++) = x;
	return;
}

inline void DataADC::set_freq(const int& freq) {
	freq_ = freq;
	return;
}

inline unsigned int DataADC::get_freq(void) const {
	return freq_;
}

} /* namespace mad_n */

#endif /* DATAADC_H_ */
