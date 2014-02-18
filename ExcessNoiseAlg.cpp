/*
 * ExcessNoiseAlg.cpp
 *
 *  Created on: 18 февр. 2014 г.
 *      Author: andrej
 */

#include "ExcessNoiseAlg.h"

namespace mad_n {

ExcessNoiseAlg::ExcessNoiseAlg(std::string name, const int& id,
		void (*pf)(void*, size_t, int)) :
		Algorithm(name, id, pf) {

}

void ExcessNoiseAlg::open_(void) {
	Algorithm::open__();
	end_ = std::async(std::launch::async, &ExcessNoiseAlg::excessNoise, this);
	return;
}

void ExcessNoiseAlg::excessNoise(void) {
	DataADC d;
	for (;;) {
		if (!check_valid_therad())
			break;
		d = *pop_fifo();
	}
	clear_fifo();
	return;
}

ExcessNoiseAlg::~ExcessNoiseAlg() {
}

} /* namespace mad_n */
