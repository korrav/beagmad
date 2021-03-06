/*
 * ContinueAlg.h
 *
 *  Created on: 02 февр. 2014 г.
 *      Author: andrej
 */

#ifndef CONTINUEALG_H_
#define CONTINUEALG_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Algorithm.h"

namespace mad_n {

/*
 *Данный класс реализует алгоритм непрерывной передачи данных на БЦ
 */
class ContinueAlg: public Algorithm {
	unsigned int numberOfSampl_; //количество отсчётов в одном пакете
	std::vector<int8_t> buf_;
public:
	bool open_(void);
	void continious(void);
	ContinueAlg(std::string name, const int& id, void (*pf)(std::vector<int8_t>&, int));
	virtual ~ContinueAlg();
};

} /* namespace mad_n */

#endif /* CONTINUEALG_H_ */
