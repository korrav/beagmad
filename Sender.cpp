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

namespace mad_n {

int Sender::sock_;
int Sender::id_;
sockaddr_in Sender::addrBag_;
std::mutex Sender::mut_;

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
	h_package head = { id_, id_block };
	char* package = new char[sizeof(h_package) + size];
	memcpy(package, &head, sizeof(head));
	memcpy(package + sizeof(head), pbuf, size);
	sendto(sock_, package, sizeof(h_package) + size, 0,
			reinterpret_cast<sockaddr*>(&addrBag_), sizeof(addrBag_));
	delete [] package;
	return;
}

Sender::~Sender() {
}

} /* namespace mad_n */
