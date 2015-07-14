/*
 * ContinueAlg.cpp
 *
 *  Created on: 02 февр. 2014 г.
 *      Author: andrej
 */

#include "ContinueAlg.h"
#include <iostream>
#include <string>
#include <string.h>
#include <cstddef>

namespace mad_n {

ContinueAlg::ContinueAlg(std::string name, const int& id, void (*pf)(std::vector<int8_t>& pbuf, int id_block)) :
												Algorithm(name, id, pf)	{
}

bool ContinueAlg::open_(void) {
	if (check_valid_theard()) {
		std::cout << "Алгоритм " << get_name() << " уже запущен\n";
		return false;
	}
	Algorithm::open__();
	end_ = std::async(std::launch::async, &ContinueAlg::continious, this);
	return true;
}

void ContinueAlg::continious(void) {
	std::shared_ptr<const DataADC> pd;
	for (;;) {
		if (!check_valid_theard())
			break;
		pd = pop_fifo();
		if (pd == nullptr)
			continue;
		size_t sizeBuf = pd->get_amount() * 4 * sizeof(short) + offsetof(DataAlgorithm, data);
		if(buf_.size() < sizeBuf)
			buf_.resize(sizeBuf);
		DataAlgorithm *pDataAlg = reinterpret_cast<DataAlgorithm*>(buf_.data());
		pDataAlg->numFirstCount = pd->get_first();
		//передача данных на берег
		memcpy(&pDataAlg->data, pd->get_data(), sizeBuf - offsetof(DataAlgorithm, data));
		pass_(&buf_, sizeBuf, CONTINIOUS, pd);
	}
	clear_fifo();
	return;
}

ContinueAlg::~ContinueAlg() {
	return;
}

} /* namespace mad_n */
