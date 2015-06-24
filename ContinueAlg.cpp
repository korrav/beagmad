/*
 * ContinueAlg.cpp
 *
 *  Created on: 02 февр. 2014 г.
 *      Author: andrej
 */

#include "ContinueAlg.h"
#include <iostream>
#include <string>
#include <string.h>

namespace mad_n {

ContinueAlg::ContinueAlg(std::string name, const int& id,
		void (*pf)(void*, size_t, int), unsigned num) :
		Algorithm(name, id, pf), numberOfSampl_(NUM_SAMPL_PACK) {
	if (num < NUM_SAMPL_PACK)
		numberOfSampl_ = num;
	else
		std::cout
				<< "Алгоритм cont: Не возможно передать пакет с количеством отсчётов данных "
				<< num << std::endl
				<< "Максимальное количество данных, которое может быть переданно в одном пакете "
				<< NUM_SAMPL_PACK << std::endl;
	return;

}

bool ContinueAlg::open_(void) {
	if (check_valid_theard()) {
		std::cout << "Алгоритм " << get_name() << " уже запущен\n";
		return false;
	}
	Algorithm::open__();
	end_ = std::async(std::launch::async, &ContinueAlg::continious, this);
	return true;
}

void ContinueAlg::continious(void) {
	DataADC d;
	std::shared_ptr<const DataADC> pd;
	for (;;) {
		if (!check_valid_theard())
			break;
		pd = pop_fifo();
		if (pd == nullptr)
			continue;
		d = *pd;
		buf_.freq = d.get_freq();
		d.get_gain(buf_.gain);
		buf_.numFirstCount = d.get_first();
		unsigned int num = d.get_amount();
		//передача данных на берег
		while (num > 0) {
			int trans = 0;
			if (num <= numberOfSampl_)
				trans = num;
			else
				trans = numberOfSampl_;
			buf_.amountCount = trans;
			memcpy(buf_.sampl,
					reinterpret_cast<short*>(d.get_data())
							+ (d.get_amount() - num) * 4,
					trans * 4 * sizeof(short));
			pass_(&buf_,
					sizeof(buf_) - (NUM_SAMPL_PACK - trans) * 4 * sizeof(short),
					CONTINIOUS_ALG);
			buf_.numFirstCount += trans;
			num -= trans;
		}
	}
	clear_fifo();
	return;
}

ContinueAlg::~ContinueAlg() {
	return;
}

} /* namespace mad_n */
