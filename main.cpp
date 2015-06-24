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
#include "gpio_overload.h"
#include "sys/poll.h"
#include <exception>

using namespace std;
using mad_n::Mad;

void hand_command_line(Mad& mad, istream& stream); //обработка инструкций командной строки

int main(int argc, char* argv[]) {
	std::string nameFileConfig = FILE_CONFIG_MAD;	//содержит имя конфигурационного файла Мада
	if (!(argc > 2 && argc < 5))
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
	if(argc > 3)
		nameFileConfig = argv[4];
	//открытие файла цифрового ввода, сигнализирующего о перегрузке pga
	int fd_over = open_file_gpio_overload();
	if (fd_over < 0)
		return 1;
	mad_n::Sender sender(sock, BAG_ADDR, BAG_PORT, atoi(argv[1]));
	cout << "Создан объект класса Sender" << std::endl;
	mad_n::SinkAdcData sink(DEV_SPI);
	cout << "Создан объект класса SinkAdcData" << std::endl;
	mad_n::ManagerAlg manager(&sink, mad_n::Sender::pass);
	cout << "Создан объект класса ManagerAlg" << std::endl;
	Mad mad(sock, mad_n::Sender::pass, &manager, &sink);
	cout << "Создан объект класса Mad" << std::endl;

	//обработка командного файла
	if (argc >= 3) {
		ifstream file(argv[2]);
		if (!file.is_open()) {
			cout << "Невозможно открыть командный файл " << argv[2]
																 << std::endl;
			return 1;
		}
		while (!file.eof())
			hand_command_line(mad, file);
		file.close();
	}

	int status = 0;
	pollfd fds[3];
	fds[0].fd = STDIN_FILENO;
	fds[0].events = POLLIN;
	fds[1].fd = sock;
	fds[1].events = POLLIN;
	fds[2].fd = fd_over;
	fds[2].events = POLLPRI;
	char buf[2];
	read(fd_over, buf, 2);
	for (;;) {
		//ожидание событий
		status = poll(fds, 3, -1);
		if (status < 0) {
			if (errno == EINTR)
				continue;
			else {
				perror("Функция poll завершилась крахом\n");
				exit(1);
			}
		}
		if (fds[0].revents & POLLIN)
			hand_command_line(mad, cin);
		if (fds[2].revents & POLLPRI) {
			mad.post_overload();
			char buf[2];
			read(fd_over, buf, 2);
		}
		if (fds[1].revents & POLLIN) {
			sockaddr_in srcAddr;
			size_t size = sizeof(srcAddr);
			unsigned len = recvfrom(sock, reinterpret_cast<void *>(recBuf),
					sizeof(recBuf), 0, reinterpret_cast<sockaddr*>(&srcAddr),
					&size);
			mad.receive(len, recBuf);
		}
	}
	return 0;
}

void hand_command_line(Mad& mad, istream& stream) {
	std::istringstream message;
	string command, mes;
	if (!getline(stream, command))
		return;
	message.str(command);
	try {
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
			} else if (mes == "set_p_mp") {
				message >> mes;
				unsigned char period = static_cast<unsigned char>(stoi(mes));
				mad.set_period_monitoring_pga(period);
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
			} else if (mes == "set_sigma") {
				message >> mes;
				int num = stoi(mes);
				mad.algExN_->set_sigma(num);
			} else if (mes == "set_par_b") {
				message >> mes;
				unsigned int bef = stoi(mes) * 4;
				message >> mes;
				unsigned int aft = stoi(mes) * 4;
				if (!mad.algExN_->set_parameter_block(bef, aft))
					std::cout
					<< "Не удалось установить новые параметры пакета, передаваеого в режиме Гасик\n";
			} else if (mes == "get_par_b") {
				unsigned int bef, aft;
				if (mad.algExN_->get_parameter_block(&bef, &aft))
					cout << "Параметры блока данных алгоритма "
					<< mad.algExN_->get_name()
					<< " : количество отсчётов до события = " << bef / 4
					<< " после события = " << aft / 4 << std::endl;
			} else if (mes == "get_ac") {
				list<std::string> listA;
				std::string algorithms = "неактивен ни один алгортм";
				mad.get_list_active_algoritm(&listA);
				if (!listA.empty()) {
					algorithms.clear();
					for (std::string& name : listA)
						algorithms += name + " ";
				}
				std::cout << "На данный момент " << algorithms << std::endl;
			} else if (mes == "en_wb_noise")
				mad.algExN_->enable_write_full_block();
			else if (mes == "set_p_dm") {
				message >> mes;
				unsigned int second = static_cast<unsigned>(stoi(mes));
				mad.set_period_monitor(second);
			} else if (mes == "get_p_dm")
				std::cout << "Период передачи мониторограмм на БЦ составляет "
				<< mad.get_period_monitor() << " секунд\n";
			else if (mes == "get_sigma")
				std::cout << "Коэфициент превышения порога шума равен "
				<< mad.algExN_->get_sigma() << std::endl;
			else
				cout << "Передана неизвестная команда\n";
		}
	} catch (...) {
		std::cout << "Неверный формат команды\n";
	}
	message.clear();
	return;
}
