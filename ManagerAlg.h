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
#include <chrono>
#include <list>

namespace mad_n {
class Monitor {
	int rms_[4]; 	//СКО
	int mean_[4];	//математическое ожидание
	mutable std::mutex mut_; //блокирует обращение к статистике
	void set_statistics(const int* rms, const int* mean); //установить статистику
	const int AMOUNT_AN = 100000; //количество отсчётов, которое учитывается при расчёте статистики
	std::chrono::steady_clock::time_point transferTime_; //время передачи мониторограммы на БЦ
	std::chrono::seconds periodTransfer_ = std::chrono::seconds(60); //значение периода передачи мониторограмм
	monitorogramm bufM_; //буфер, содержащий мониторограмму, предназначенную для передачи на БЦ
	void (*pass_)(void* pbuf, size_t size, int id_block);
public:
	void set_period(const unsigned int& s); //установить период передачи мониторограмм
	unsigned int get_period(void); //получить период передачи мониторограмм
	void get_statistics(int* rms, int* mean) const; //получить статистику
	void calculation_stats(const DataADC& bData);	//вычислить статистику
	Monitor(void (*pf)(void*, size_t, int));
	~Monitor() {
		mut_.unlock();
	}
};
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
	Monitor monitor_; //объект расчёта мониторограмм
	void get_active_algorithm(std::list<std::string>* pls, std::list<int>* pli); //получить наименования и идентификаторы активных алгоритмов
	bool addAlgorithm(Algorithm* a); //добавление алгоритма в набор алгоритмов менеджера алгоритмов
	void get_rms(int* prms); //получить текущее значение rms
	void get_mean(int* pmean); //получить текущее значение математического ожидания
	bool turnOn(const std::string& alg); //включить алгоритм
	bool turnOn(const int& alg); //включить алгоритм
	bool turnOff(const std::string& alg); //выключить алгоритм
	bool turnOff(const int& alg); //выключить алгоритм
	void turnOffAll(void); //выключить все действующие алгоритмы
	void set_task_in(const std::string nameAlg, const std::string& nameFile,
			const int& num = WriteDataToFile::SIZE_P); //установить задание на запись данных входной очереди алгоритма
	void set_task_out(const std::string nameAlg, const std::string& nameFile,
			const int& num = WriteDataToFile::SIZE_P); //установить задание на запись данных выходной очереди алгоритма
	int get_count_queue(const std::string& name); //возвратить количество элементов в очереди
	ManagerAlg(SinkAdcData* s, void (*pf)(void*, size_t, int));
	virtual ~ManagerAlg();
};

inline void Monitor::set_period(const unsigned int& s) {
	periodTransfer_ = std::chrono::seconds(s);
	return;
}

inline unsigned int Monitor::get_period(void) {
	return periodTransfer_.count();
}

} /* namespace mad_n */

#endif /* MANAGERALG_H_ */
