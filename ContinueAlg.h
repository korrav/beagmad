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
	datA buf_;
public:
	void open_(void);
	void continious(void);
	ContinueAlg(std::string name, const int& id, void (*pf)(void*, size_t, int), unsigned num = 1000);
	virtual ~ContinueAlg();
};

} /* namespace mad_n */

#endif /* CONTINUEALG_H_ */
