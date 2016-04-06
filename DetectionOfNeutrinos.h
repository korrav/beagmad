/*
 * DetectionOfNeutrinos.h
 *
 *  Created on: 18 июля 2015 г.
 *      Author: andrej
 */

#ifndef DETECTIONOFNEUTRINOS_H_
#define DETECTIONOFNEUTRINOS_H_
#include <vector>
#include "Algorithm.h"
#include <string>

namespace mad_n {
/*
 *класс, реализующий алгоритм по выделению нейтриноподобных событий
 */
class DetectionOfNeutrinos: public Algorithm {
	class excep_out_of_algorithm { //класс, используемый для выхода из главной функции алгоритма
	};
	Filter* buf_;
	std::vector<short> sampl_;	//отсчёты в пуле
	std::shared_ptr<const DataADC> pd_ = nullptr;

	void detection(void); //основная подпрограмма детектирования нейтрино
	void transfer_data(std::vector<short>::iterator & pos, const size_t &num_sampl) const;	//передача данных на берег
public:
	bool open_(void);
	DetectionOfNeutrinos(std::string name, const int& id,void (*pf)(std::vector<int8_t>&, int));
	virtual ~DetectionOfNeutrinos();
};

} /* namespace mad_n */

#endif /* DETECTIONOFNEUTRINOS_H_ */
