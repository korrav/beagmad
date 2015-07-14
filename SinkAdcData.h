/*
 * SinkAdcData.h
 *
 *  Created on: 01 февр. 2014 г.
 *      Author: andrej
 */

#ifndef SINKADCDATA_H_
#define SINKADCDATA_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <deque>
#include <mutex>
#include <future>
#include <condition_variable>
#include <memory>
#include <algorithm>
#include "DataADC.h"
#include "WriteDataToFile.h"

namespace mad_n {

/*
 *Объект данного класса принимает данные от АЦП и помещает их в очередь для дальнейшей обработки
 */
class SinkAdcData {
	typedef std::deque<std::unique_ptr<DataADC> > deqU;
	deqU fifo_; //очередь блоков данных АЦП
	int f_adc_; //дескриптор файла АЦП
	std::future<void> end_; //будущий результат, возвращаемый только после уничтожения потока алгоритма
	bool isRunThread_;	//поток запущен
	std::mutex mut_; //регулирует доступ к очереди блоков АЦП
	std::condition_variable isEmpty_; //условная переменная, используемая при проверке пуста ли очередь блоков АЦП
	short gain_[4]; //текущие коэффициенты усиления
	int freq_; //частота дискретизации(в Гц)
	WriteDataToFile wdtIn_; // объект обеспечивающий запись входных данных

	void openMain(void); //открытие главного потока
	void closeMain(void); //закрытие главного потока
	void main(void); //главный поток объекта
public:
	std::shared_ptr<DataADC> obtainData(void);
	int get_count_queue(void); //возвратить количество элементов в очереди
	void set_gain(short* g); //установка коэффициентов усиления каналов
	void get_gain(int32_t* g); //получение коэффициентов усиления каналов
	void set_freq(const int& f); //установка частоты дискретизации
	int get_freq(void); //получение частоты дискретизации
	void set_task_in(const std::string& nameFile, const int& num);
	SinkAdcData(const char* dev);
	virtual ~SinkAdcData();
};

} /* namespace mad_n */

#endif /* SINKADCDATA_H_ */
