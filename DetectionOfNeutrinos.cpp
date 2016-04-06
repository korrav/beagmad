/*
 * DetectionOfNeutrinos.cpp
 *
 *  Created on: 18 июля 2015 г.
 *      Author: andrej
 */

#include "DetectionOfNeutrinos.h"
#include <iostream>
#include <algorithm>
#include "recognize.h"
#include "TData.h"

#define LEN_WINDOW (SECTION_LENGTH * 4)	 //длина измерительного окна

namespace mad_n {

DetectionOfNeutrinos::DetectionOfNeutrinos(std::string name, const int& id,void (*pf)(std::vector<int8_t>&, int)): Algorithm(name, id, pf)  {
	pBuf_ = new int8_t[sizeof(Filter) + LEN_WINDOW * sizeof(short)];
}

bool DetectionOfNeutrinos::open_(void) {
	if (check_valid_theard()) {
		std::cout << "Алгоритм " << get_name() << " уже запущен\n";
		return false;
	}
	Algorithm::open__();
	end_ = std::async(std::launch::async, &DetectionOfNeutrinos::detection, this);
	return true;
}

void DetectionOfNeutrinos::detection(void) {
	std::vector<short>::iterator curPos; //текущая позиция
	unsigned int numFirstCount = 0;
	std::vector<unsigned> buf(LEN_WINDOW);
	for (;;) {
		if (!check_valid_theard())
			break;
		pd_ = pop_fifo();
		if(pd_ == nullptr)
			continue;
		//отсчёты сигналов копируются в пул
		std::copy_n(pd_->get_buf(), pd_->get_amount() * 4, std::back_inserter(sampl_));
		curPos = sampl_.begin();
		//непосредственно фильтрация
		while ((curPos + LEN_WINDOW)< sampl_.end()) {
			std::copy_n(curPos, LEN_WINDOW, buf.begin());
			if (SectionHasNeutrinoLikePulse(buf.data() /*, LEN_WINDOW*/, true)) {	//нейтрино обнаружен
				//передача куска данных на берег
				transfer_data(curPos, LEN_WINDOW);
				curPos += LEN_WINDOW;
			} else
				curPos += LEN_WINDOW / 2;
		}
		//обрезание пула
		sampl_.erase(sampl_.begin(), curPos);
	}
	clear_fifo();
	return;
}

void DetectionOfNeutrinos::transfer_data(std::vector<short>::iterator& pos, const size_t& num_sampl) const {
	buf_.param.sigma = static_cast<float>(SIGMA_LEVEL);
	std::copy_n(pos, num_sampl, reinterpret_cast<short*>(&buf_.buf.data));
	size_t sizeBuf = num_sampl * sizeof(short) + offsetof(Filter, buf) + offsetof(DataAlgorithm, data);
	if(pd_ != nullptr)
		pass_(pBuf_, sizeBuf, FILTER, pd_);
	return;
}

DetectionOfNeutrinos::~DetectionOfNeutrinos() {
	delete[] reinterpret_cast<int8_t*>(pBuf_);
}

} /* namespace mad_n */
