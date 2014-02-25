#include "main.h"
#include "Sender.h"
#include "SinkAdcData.h"
#include "Mad.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include "WriteDataToFile.h"

using namespace std;
using mad_n::Mad;

void hand_command_line(Mad& mad); //обработка инструкций командной строки

int main(int argc, char* argv[]) {
	if (argc != 2)
		return 1;
	int recBuf[SIZE_REC_BUF]; //приёмный буфер
	cout << "Привет миру от Andrej!" << endl;
	int sock;	//сокет МАД
	//инициализация адреса МАД
	sockaddr_in addrMad;
	bzero(&addrMad, sizeof(addrMad));
	addrMad.sin_family = AF_INET;
	addrMad.sin_port = htons(MAD_PORT);
	addrMad.sin_addr.s_addr = htonl(INADDR_ANY);
	//создание сокета
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1) {
		std::cerr << "создать сокет не удалось\n";
		return 1;
	}
	if (bind(sock, reinterpret_cast<sockaddr*>(&addrMad), sizeof(addrMad))) {
		std::cerr << "не удалось связать сокет с адресом\n";
		return 1;
	}
	int sizeSend = 2 * MAX_SIZE_SAMPL_SEND * 4 * sizeof(short);
	if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sizeSend, sizeof(int)) == -1) {
		std::cerr
				<< "Не поддерживается объём буфера передачи сокета  в размере "
				<< sizeSend << " байт\n";
		return 1;
	}
	mad_n::Sender sender(sock, BAG_ADDR, BAG_PORT, atoi(argv[1]));
	cout << "Создан объект класса Sender" << std::endl;
	mad_n::SinkAdcData sink(DEV_SPI);
	cout << "Создан объект класса SinkAdcData" << std::endl;
	mad_n::ManagerAlg manager(&sink, mad_n::Sender::pass);
	cout << "Создан объект класса ManagerAlg" << std::endl;
	Mad mad(sock, mad_n::Sender::pass, &manager, &sink);
	cout << "Создан объект класса Mad" << std::endl;
	fd_set fdin; //набор дескрипторов, на которых ожидаются входные данные
	int status = 0;

	for (;;) {
		FD_ZERO(&fdin);
		FD_SET(STDIN_FILENO, &fdin);
		FD_SET(sock, &fdin);
		//ожидание событий
		status = select(sock + 1, &fdin, NULL, NULL, NULL);
		if (status == -1) {
			if (errno == EINTR)
				continue;
			else {
				perror("Функция select завершилась крахом\n");
				exit(1);
			}
		}
		if (FD_ISSET(STDIN_FILENO, &fdin))
			hand_command_line(mad);
		if (FD_ISSET(sock, &fdin)) {
			sockaddr_in srcAddr;
			size_t size = sizeof(srcAddr);
			unsigned len = recvfrom(sock, reinterpret_cast<void *>(&recBuf),
					sizeof(recBuf), 0, reinterpret_cast<sockaddr*>(&srcAddr),
					&size);
			mad.receive(len, &recBuf);
		}
	}
	return 0;
}

void hand_command_line(Mad& mad) {
	std::istringstream message;
	string command, mes;
	getline(cin, command);
	message.str(command);
	while (message >> mes) {
		if (mes == "start")
			mad.start_adc();
		else if (mes == "stop")
			mad.stop_adc();
		else if (mes == "reg")
			mad.read_reg();
		else if (mes == "spi0")
			mad.start_spi0();
		else if (mes == "spi1")
			mad.start_spi1();
		else if (mes == "exit")
			exit(0);
		else if (mes == "startt")
			mad.start_test();
		else if (mes == "stopt")
			mad.stop_test();
		else if (mes == "gain") {
			int buf[4];
			for (int i = 3; i >= 0; i--) {
				message >> mes;
				buf[i] = stoi(mes);
			}
			mad.set_gain(buf);
		} else if (mes == "open_a") {
			message >> mes;
			mad.open_alg(mes);
		} else if (mes == "close_a") {
			message >> mes;
			mad.close_alg(mes);
		} else if (mes == "close_aa")
			mad.close_all_alg();
		else if (mes == "writin") {
			int num = mad_n::WriteDataToFile::SIZE_P;
			string nAlg, nFile;
			message >> nAlg >> nFile;
			message >> mes;
			if (mes != "p")
				num = stoi(mes);
			mad.set_task_in(nAlg, nFile, num);
		} else if (mes == "writout") {
			int num = mad_n::WriteDataToFile::SIZE_P;
			string nAlg, nFile;
			message >> nAlg >> nFile;
			message >> mes;
			if (mes != "p")
				num = stoi(mes);
			mad.set_task_out(nAlg, nFile, num);
		} else if (mes == "n_elem") {
			message >> mes;
			mad.get_count_queue(mes);
		} else
			cout << "Передана неизвестная команда\n";
	}
	message.clear();
	return;
}
