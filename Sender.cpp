/*
 * Sender.cpp
 *
 *  Created on: 02 февр. 2014 г.
 *      Author: andrej
 */

#include "Sender.h"
#include "wraping.h"
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <algorithm>
namespace mad_n {

int Sender::sock_;
int Sender::id_;
sockaddr_in Sender::addrBag_;
std::mutex Sender::mut_;
int Sender::idPack_ = 0;

Sender::Sender(const int& sock, const char* ip, const unsigned short& port,
		const int& id) {
	sock_= sock;
	id_ = id;
	bzero(&addrBag_, sizeof(addrBag_));
	addrBag_.sin_family = AF_INET;
	addrBag_.sin_port = htons(port);
	inet_pton(AF_INET, ip, &addrBag_.sin_addr);
}

void Sender::pass(void* pbuf, size_t size, int id_block) {
	std::lock_guard<std::mutex> lock(mut_);

	//получение идентификатора передаваемого пакета
	static std::array<int8_t, MAX_SIZE_PACK> buf;
	const int MAX_SIZE_PAYLOAD_PACK = MAX_SIZE_PACK - sizeof(HeadPack);
	static HeadPack* pHead = reinterpret_cast<HeadPack*>(&buf[0]);
	static auto pData = buf.begin() + sizeof(HeadPack);

	int8_t* pTransBuf = reinterpret_cast<int8_t*>(pbuf);
	//из скольких частей состоит пакет
	int numPart = ceil(static_cast<double>(size)/ MAX_SIZE_PAYLOAD_PACK);
	//заполнение шапки пакета
	pHead->endpoints.idSrc = id_;
	pHead->endpoints.idBlock = id_block;
	pHead->mark.id = getIdPack();
	pHead->mark.totalPart = numPart;
	pHead->mark.part = 0;
	//передача
	int sizePayOfCurPack = 0;
	while(pHead->mark.part < numPart) {
		if(pHead->mark.part == numPart - 1)
			sizePayOfCurPack = size;
		else
			sizePayOfCurPack = MAX_SIZE_PAYLOAD_PACK;
		size -= sizePayOfCurPack;
		std::copy_n(pTransBuf + pHead->mark.part * MAX_SIZE_PAYLOAD_PACK, sizePayOfCurPack, pData);
		sendto(sock_, &buf[0], sizePayOfCurPack + sizeof(HeadPack), 0, reinterpret_cast<sockaddr*>(&addrBag_), sizeof(addrBag_));
		pHead->mark.part++;
	}
	return;
}

Sender::~Sender() {
}

} /* namespace mad_n */
