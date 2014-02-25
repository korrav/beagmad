/*
 * ExcessNoiseAlg.cpp
 *
 *  Created on: 18 февр. 2014 г.
 *      Author: andrej
 */

#include "ExcessNoiseAlg.h"
#include <iostream>

namespace mad_n {

ExcessNoiseAlg::ExcessNoiseAlg(std::string name, const int& id,
		void (*pf)(void*, size_t, int)) :
		Algorithm(name, id, pf) {

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
	DataADC d;
	std::shared_ptr<const DataADC> pd;
	for (;;) {
		if (!check_valid_therad())
			break;
		pd = pop_fifo();
		if (pd == nullptr)
			continue;
		d = *pd;
	}
	clear_fifo();
	return;
}

ExcessNoiseAlg::~ExcessNoiseAlg() {
}

} /* namespace mad_n */
