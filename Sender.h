/*
 * Sender.h
 *
 *  Created on: 02 февр. 2014 г.
 *      Author: andrej
 */

#ifndef SENDER_H_
#define SENDER_H_
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mutex>
#include "wraping.h"

namespace mad_n {

/*
 *
 */
class Sender {
	static int sock_;	//сокет БЦ
	static int id_;	//идентификатор МАД
	static sockaddr_in addrBag_; //адрес БЭГ
	static std::mutex mut_;	//мьютекс для защиты передачи данных
public:
	static void pass(void* pbuf, size_t size, int id_block);
	Sender(const int& sock, const char* ip, const unsigned short& port,
			const int& id);
	virtual ~Sender();
};

} /* namespace mad_n */

#endif /* SENDER_H_ */
