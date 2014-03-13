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
#include <vector>
#include <iostream>
#include "Algorithm.h"

namespace mad_n {

class ManagerAlg;

/*
 *класс, реализующий алгоритм по превышению уровня шумов
 */
class ExcessNoiseAlg: public Algorithm {
	class excep_out_of_algorithm { //класс, используемый для выходя из главной функции алгоритма
	};
	unsigned int numFirstCount_; //номер первого отсчёта в приёмном буфере
	datA buf_;
	const unsigned int INIT_BEFORE_EVENT = 100; //начальное значение параметра количество отсчётов до события
	const unsigned int INIT_AFTER_EVENT = 1800; //начальное значение параметра количество отсчётов до события
	const short INIT_SIGMA = 6; //начальное значение параметра количество отсчётов до события
	unsigned int beforeEvent_; //количество отсчётов до события
	unsigned int afterEvent_; //количество отсчётов после события (включая момент события)
	short sigma_; //коэффициент превышения шумов (определяет необходимый уровень превышения шумов)
	ManagerAlg* man_; //указатель на объект накопления статистики
	mutable std::mutex mutBefAf_; //мьютекс, защищающий модификацию параметров передаваемого блока данных
	static std::shared_ptr<const DataADC> pd_; //указывает на обрабатываемый в данный момент блок
	bool isEnableWriteFullBlock_; //разрешение записать целый блок данных, в котором было обнаружено превышение порога шума
	const std::string NameFileFullBlock_ = "./full_block_gasik"; //имя файла, в который будет записана информация блока, в котором было обнаружено превышение шума
	void searchIncreaseNoise(std::vector<short>::iterator& cur,
			const std::vector<short>::iterator& end, int* rms, int* mean); //функция поиска отсчёта, превышающего по значению уровень шумов
	void increase_size_buf(std::vector<short>& buf, unsigned int size); //увеличить размер буфера, чтобы его значение превысило size элементов
	void transfer_data(std::vector<short>& v,
			const std::vector<short>::iterator& cur, const unsigned &num_sampl); //передача данных на БЦ
	void writeFullBlock(void); //запись текущего блока
public:
	void enable_write_full_block(void); //разрешить записать целый блок данных, в котором было обнаружено превышение порога шума
	bool set_parameter_block(const unsigned int& bef,
			const unsigned int& after); //установить параметры блока данных, передаваемого на БЦ
	bool get_parameter_block(unsigned int *bef, unsigned int *after) const; //получить параметры блока данных, передаваемого на БЦ
	void set_sigma(const short& s); //установить коэффициент превышения шума
	bool open_(void);
	void excessNoise(void);
	ExcessNoiseAlg(std::string name, const int& id,
			void (*pf)(void*, size_t, int), ManagerAlg* man);
	virtual ~ExcessNoiseAlg();
};

inline void ExcessNoiseAlg::set_sigma(const short & s) {
	if (!check_valid_therad()) {
		std::cout << " Алгоритм " << get_name() << " не активен\n";
		return;
	}
	sigma_ = s;
	return;
}

inline bool ExcessNoiseAlg::set_parameter_block(const unsigned int& bef,
		const unsigned int& after) {
	std::lock_guard<std::mutex> lk(mutBefAf_);
	if (bef + after > NUM_SAMPL_PACK)
		return false;
	beforeEvent_ = bef;
	afterEvent_ = after;
	return true;
}

inline bool ExcessNoiseAlg::get_parameter_block(unsigned int* bef,
		unsigned int* after) const {
	std::lock_guard<std::mutex> lk(mutBefAf_);
	*bef = beforeEvent_;
	*after = afterEvent_;
	return true;
}

inline void ExcessNoiseAlg::enable_write_full_block(void) {
	isEnableWriteFullBlock_ = true;
	std::cout
			<< "Позволена запись одного блока данных, в котором обнаружено превышение над уровнем сигнала\n";
	return;
}

} /* namespace mad_n */

#endif /* EXCESSNOISEALG_H_ */
