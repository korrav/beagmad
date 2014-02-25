/*
 * ExcessNoiseAlg.h
 *
 *  Created on: 18 февр. 2014 г.
 *      Author: andrej
 */

#ifndef EXCESSNOISEALG_H_
#define EXCESSNOISEALG_H_
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Algorithm.h"

namespace mad_n {

/*
 *класс, реализующий алгоритм по превышению уровня шумов
 */
class ExcessNoiseAlg: public Algorithm {
	datA buf_;
public:
	bool open_(void);
	void excessNoise(void);
	ExcessNoiseAlg(std::string name, const int& id,
			void (*pf)(void*, size_t, int));
	virtual ~ExcessNoiseAlg();
};

} /* namespace mad_n */

#endif /* EXCESSNOISEALG_H_ */
