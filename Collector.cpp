/*
 * Collector.cpp
 *
 *  Created on: 07 мая 2015 г.
 *      Author: andrej
 */

#include "Collector.h"
#include <iostream>
#include <algorithm>

namespace collector_n {

Collector::Collector(): pBuf_(new PtrData::element_type()) {
	pCurMark_ = new MarkPack;
	init();
}

void Collector::init(void) {
	pBuf_->clear();
	pCurMark_->id = -1;
	pCurMark_->part = -1;
	pCurMark_->totalPart = -1;
}

std::pair<PtrData, int> Collector::receivBlock(PtrData pDataBuf) {
	if(pDataBuf->size() < sizeof(HeadPack))
		return {nullptr, -1};
	int idBlock = reinterpret_cast<HeadPack*>(pDataBuf->data())->endpoints.idBlock;
	MarkPack* pMark = reinterpret_cast<MarkPack*>(pDataBuf->data() + sizeof(SrcPack));
	if((pCurMark_->id != pMark->id) || (pMark->part == 0)) {	//пришёл пакет с новым идентификатором блока
		if(pCurMark_->part != -1)
			init();
		if(pMark->part == 0) {
			pCurMark_->id = pMark->id;
			pCurMark_->totalPart = pMark->totalPart;
		} else {
			init();
			return {nullptr, -1};
		}
	}
	if(++pCurMark_->part != pMark->part) {
		init();
		return {nullptr, -1};
	}
	std::copy(pDataBuf->begin() + sizeof(HeadPack),pDataBuf->end(),std::back_inserter(*pBuf_));
	if(pCurMark_->part == pCurMark_->totalPart - 1) {
		PtrData pNewBuf(new PtrData::element_type());
		pBuf_.swap(pNewBuf);
		init();
		return {pNewBuf, idBlock};
	} else
		return {nullptr, -1};
}

Collector::~Collector() {
	delete pCurMark_;
}

} /* namespace collector_n */
