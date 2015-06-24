/*
 * Algorithm.h
 *
 *  Created on: 01 февр. 2014 г.
 *      Author: andrej
 */

#ifndef ALGORITHM_H_
#define ALGORITHM_H_
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
#include "wraping.h"
#include "WriteDataToFile.h"

namespace mad_n {

/*
 *Данный класс является базовым для остальных классов-алгоритмов
 */
class Algorithm {
	bool isRunThread_ = false;	//поток запущен
	std::condition_variable isEmptyAndClose_; //условная переменная, используемая при проверке пуста ли очередь блоков АЦП
	std::deque<std::shared_ptr<const DataADC> > fifo_; //входная очередь потока
	std::mutex mut_fifo_; //мьютекс для управления очередью
	std::string name_; //имя алгоритма
	int id_; //числовой идентификатор алгоритма
	void (*pass__)(void* pbuf, size_t size, int id_block);
	WriteDataToFile wdtOut_; // объект обеспечивающий запись выходных данных
	WriteDataToFile wdtIn_; // объект обеспечивающий запись входных данных
public:
	virtual bool open_(void) = 0; //открытие потока (данная функция обязательно должна быть вызвана при открытии потока)
	inline int get_id(void); //возвращает идентификатор алгоритма
	void close_(void); //закрытие потока
	inline bool check_valid_theard(void) const { //проверка активен ли поток алгоритма
		return isRunThread_;
	}
	inline std::string get_name(void) const { //получить имя алгоритма
		return name_;
	}
	int get_count_queue(void); //возвратить количество элементов в очереди
	void push_fifo(std::shared_ptr<const DataADC> d); //помещения блока данных во входную очередь потока
	void clear_fifo(void); //опустошение входной очереди
	std::shared_ptr<const DataADC> pop_fifo(void); //изъятие из потока блока данных
	void set_task_in(const std::string& nameFile, const int& num =
			WriteDataToFile::SIZE_P); //установить задание на запись данных входной очереди алгоритма
	void set_task_out(const std::string& nameFile, const int& num =
			WriteDataToFile::SIZE_P); //установить задание на запись данных выходной очереди алгоритма
	Algorithm(std::string name, const int& id, void (*pf)(void*, size_t, int));
	virtual ~Algorithm();
protected:
	std::future<void> end_; //будущий результат, возвращаемый только после уничтожения потока алгоритма
	void pass_(void* pbuf, size_t size, int id_block);
	void open__(void) { //эта функция должна быть вызвана в функции open_
		isRunThread_ = true;
		return;
	}
};

inline int Algorithm::get_id(void) {
	return id_;
}

} /* namespace mad_n */

#endif /* ALGORITHM_H_ */
