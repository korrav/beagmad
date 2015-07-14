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
#include <vector>
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
	static int idPack_;	//идентификатор передаваемого пакета данных
	static const int MAX_SIZE_PACK = 1400;	//максимальный размер передаваемого UDP пакета
public:
	static void pass(std::vector<int8_t>& pbuf, int id_block);
	Sender(const int& sock, const char* ip, const unsigned short& port,
			const int& id);
	static inline int getIdPack(void) {
		if(++idPack_ >= std::numeric_limits<int>::max())
			idPack_ = 0;
		return idPack_;
	}
	virtual ~Sender();
};

} /* namespace mad_n */

#endif /* SENDER_H_ */
