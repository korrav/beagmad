/*
 * ManagerAlg.h
 *
 *  Created on: 01 февр. 2014 г.
 *      Author: andrej
 */

#ifndef MANAGERALG_H_
#define MANAGERALG_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mutex>
#include <future>
#include <memory>
#include <map>
#include <algorithm>
#include "DataADC.h"
#include "SinkAdcData.h"
#include "Algorithm.h"

namespace mad_n {

/*
 *
 */
class ManagerAlg {
	typedef std::map<std::string, Algorithm*> AlgMap;
	std::future<void> end_; //будущий результат, возвращаемый только после уничтожения потока алгоритма
	bool isRunThread_;	//поток запущен
	SinkAdcData* supplier_;
	AlgMap setA_;	//набор алгоритмов
	void openDistributor(void); //открытие потока-распределителя
	void closeDistributor(void); //закрытие потока-распределителя
	void distributor(void); //поток - распределитель данных между активными алгоритмами
public:
	bool addAlgorithm(Algorithm* a); //добавление алгоритма в набор алгоритмов менеджера алгоритмов
	bool turnOn(const std::string& alg); //включить алгоритм
	bool turnOn(const int& alg); //включить алгоритм
	bool turnOff(const std::string& alg); //выключить алгоритм
	bool turnOff(const int& alg); //выключить алгоритм
	void turnOffAll(void); //выключить все действующие алгоритмы
	explicit ManagerAlg(SinkAdcData* s);
	virtual ~ManagerAlg();
};

} /* namespace mad_n */

#endif /* MANAGERALG_H_ */
