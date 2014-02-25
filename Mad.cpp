/*
 * Mad.cpp
 *
 *  Created on: 11 февр. 2014 г.
 *      Author: andrej
 */

#include "Mad.h"
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

enum i2c_id {
	START_ADC, STOP_ADC, SET_GAIN, START_SPI0, START_SPI1, START_TEST, STOP_TEST
};

//КОМАНДЫ IOCTL
#define ID_IO_F3_SPI 200	//идентификатор для команд ioctl
//запуск АЦП
#define IOCTL_ADC_START _IO(ID_IO_F3_SPI, 0)
//синхронизация для АЦП
#define IOCTL_ADC_SYNC _IO(ID_IO_F3_SPI, 1)
//остановка АЦП
#define IOCTL_ADC_STOP _IO(ID_IO_F3_SPI, 2)
//приём данных от АЦП
#define IOCTL_ADC_GET_MESSAGE _IOWR(ID_IO_F3_SPI, 0, struct dataUnit_ADC*)
//получение информации о регистрах модулей SPI0 и SPI1 в пространстве ядра
#define IOCTL_READ_REGISTERS _IO(ID_IO_F3_SPI, 5)

namespace mad_n {

bool Mad::stop_adc(void) {
	bool status = false;
	if (!isRunThreadAdc_) {
		std::cout << "Не был запущен процесс ацп преобразования\n";
		status = true;
		return status;
	}
	char buf = STOP_ADC;
	if (write(f3_i2c_, &buf, 1) == 1)
		std::cout << "Передан приказ об остановке АЦП преобразования\n";
	else {
		std::cout
				<< "Приказ об остановке АЦП преобразования передать не удалось\n";
		return status;
	}
	if (ioctl(f3_spi_, IOCTL_ADC_STOP) < 0)
		std::cout
				<< "Не удалось передать команду IOCTL_ADC_STOP в драйвер f3_spi\n";
	else {
		isRunThreadAdc_ = false;
		status = true;
	}
	return status;
}

bool Mad::start_test(void) {
	bool status = false;
	if (isRunThreadAdc_) {
		std::cout << "Процесс ацп преобразования уже запущен\n";
		return status;
	} else if (isRunThreadTest_) {
		status = true;
		std::cout << "Процесс тестового преобразования уже запущен\n";
		return status;
	}
	char buf = STOP_TEST;
	if (write(f3_i2c_, &buf, 1) != 1) {
		std::cout
				<< "Приказ об остановке тестового преобразования передать не удалось\n";
		return status;
	}
	usleep(1000);
	if (ioctl(f3_spi_, IOCTL_ADC_START) < 0) {
		std::cout
				<< "Не удалось передать команду IOCTL_ADC_START в драйвер f3_spi\n";
		return status;
	}
	buf = START_TEST;
	if (write(f3_i2c_, &buf, 1) == 1) {
		std::cout << "Передан приказ о запуск тестового преобразования\n";
		isRunThreadTest_ = true;
		status = true;
	} else
		std::cout
				<< "Приказ о запуске тестового преобразования передать не удалось\n";
	return status;
}

bool Mad::stop_test(void) {
	bool status = false;
	if (!isRunThreadTest_) {
		std::cout << "Не был запущен процесс тестового преобразования\n";
		status = true;
		return status;
	}
	char buf = STOP_TEST;
	if (write(f3_i2c_, &buf, 1) == 1)
		std::cout << "Передан приказ об остановке тестового преобразования\n";
	else {
		std::cout
				<< "Приказ об остановке тестового преобразования передать не удалось\n";
		return status;
	}
	if (ioctl(f3_spi_, IOCTL_ADC_STOP) < 0)
		std::cout
				<< "Не удалось передать команду IOCTL_ADC_STOP в драйвер f3_spi\n";

	else {
		isRunThreadTest_ = false;
		status = true;
	}
	return status;
}

bool Mad::set_gain(int* gain) {
	bool status = false;
	char buf[5] = { SET_GAIN };
	int N = 0;
	for (int i = 1; i < 5; i++) {
		if (gain[i - 1] == GAIN_)
			buf[5 - i] = 0;
		else if (gain[i - 1] >= GAIN_ + 9 && gain[i - 1] <= GAIN_ + 60) {
			N = (gain[i - 1] - GAIN_ - 6) / 3;
			gain[i - 1] = GAIN_ + 6 + 3 * N;
			buf[5 - i] = 6 + 3 * N;
		} else
			return status;
	}
	if (write(f3_i2c_, &buf, 5) == 5) {
		std::cout
				<< "Установлены новые коэффициенты усиления измерительных каналов\n";
		short shgain[4] = { static_cast<short>(gain[0]),
				static_cast<short>(gain[1]), static_cast<short>(gain[2]),
				static_cast<short>(gain[3]) };
		sinkAdc_->set_gain(shgain);
		status = true;
	} else
		std::cout
				<< "Попытка установить новые коэффициенты усиления потерпела неудачу\n";
	return status;
}

bool Mad::start_spi0(void) {
	bool status = false;
	char buf = START_SPI0;
	if (write(f3_i2c_, &buf, 1) == 1) {
		std::cout << "Передан приказ о единичной передаче spi0\n";
		status = true;
	} else
		std::cout << "Приказ о единичной передаче spi0 передать не удалось\n";
	return status;
}

bool Mad::start_spi1(void) {
	bool status = false;
	char buf = START_SPI1;
	if (write(f3_i2c_, &buf, 1) == 1) {
		std::cout << "Передан приказ о единичной передаче spi1\n";
		status = true;
	} else
		std::cout << "Приказ о единичной передаче spi1 передать не удалось\n";
	return status;
}

void Mad::read_reg(void) {
	if (ioctl(f3_spi_, IOCTL_READ_REGISTERS) < 0) { //получение данных от драйвера АЦП
		std::cout
				<< "Запрос содержимого регистров окончился неудачей неудачей\n";
	} else
		std::cout
				<< "Информация дампа памяти модуля выведена в системный лог\n";
	return;
}

void Mad::open_alg(const std::string& name) {
	manager_->turnOn(name);
	return;
}

void Mad::close_alg(const std::string& name) {
	manager_->turnOff(name);
	return;
}

bool Mad::open_alg(const int& id) {
	bool status = manager_->turnOn(id);
	std::cout << (status ? "запущен алгоритм " : "не существует алгоритма ")
			<< "с идентификаторм " << id << std::endl;
	return status;
}

bool Mad::close_alg(const int& id) {
	bool status = manager_->turnOff(id);
	std::cout << (status ? "Остановлен алгоритм " : "Не существует алгоритма ")
			<< "с идентификаторм " << id << std::endl;
	return status;
}

void Mad::close_all_alg(void) {
	manager_->turnOffAll();
	std::cout << "Остановлены все действующие алгоритмы\n";
	return;
}

void Mad::receive(const unsigned int& len, void* pbuf) {
	if (len < sizeof(h_pack_com))
		return;
	unsigned len_com = len - sizeof(h_package);
	h_pack_com* command = reinterpret_cast<h_pack_com*>(pbuf);
	int* arg = nullptr;
	if (len_com > 0)
		arg = reinterpret_cast<int*>(reinterpret_cast<char*>(pbuf)
				+ sizeof(h_pack_com));
	h_pack_ans answer = { command->id };
	switch (command->id) {
	case ID_START_ADC:
		if (len_com != sizeof(int))
			break;
		if (start_adc())
			answer.status = OK;
		else
			answer.status = NOT_OK;
		pass_(&answer, sizeof(answer), ANSWER);
		break;
	case ID_STOP_ADC:
		if (len_com != sizeof(int))
			break;
		if (stop_adc())
			answer.status = OK;
		else
			answer.status = NOT_OK;
		pass_(&answer, sizeof(answer), ANSWER);
		break;
	case ID_START_TEST:
		if (len_com != sizeof(int))
			break;
		if (start_test())
			answer.status = OK;
		else
			answer.status = NOT_OK;
		pass_(&answer, sizeof(answer), ANSWER);
		break;
	case ID_STOP_TEST:
		if (len_com != sizeof(int))
			break;
		if (stop_test())
			answer.status = OK;
		else
			answer.status = NOT_OK;
		pass_(&answer, sizeof(answer), ANSWER);
		break;
	case ID_SET_GAIN:
		if (len_com != 5 * sizeof(int))
			break;
		if (set_gain(arg))
			answer.status = OK;
		else
			answer.status = NOT_OK;
		pass_(&answer, sizeof(answer), ANSWER);
		break;
	case OPEN_ALG:
		if (len_com != 2 * sizeof(int))
			break;
		if (open_alg(*arg))
			answer.status = OK;
		else
			answer.status = NOT_OK;
		pass_(&answer, sizeof(answer), ANSWER);
		break;
	case CLOSE_ALG:
		if (len_com != 2 * sizeof(int))
			break;
		if (close_alg(*arg))
			answer.status = OK;
		else
			answer.status = NOT_OK;
		pass_(&answer, sizeof(answer), ANSWER);
		break;
	case CLOSE_ALL_ALG:
		if (len_com != sizeof(int))
			break;
		close_all_alg();
		answer.status = OK;
		pass_(&answer, sizeof(answer), ANSWER);
		break;
	}
}

void Mad::set_task_in(const std::string nameAlg, const std::string& nameFile,
		const int& num) {
	if (nameAlg == "sink")
		sinkAdc_->set_task_in(nameFile, num);
	else
		manager_->set_task_in(nameAlg, nameFile, num);
	return;
}

void Mad::set_task_out(const std::string nameAlg, const std::string& nameFile,
		const int& num) {
	if (nameAlg == "sink") {
		std::cout
				<< "Для потока приёмника данных от АЦП не предусмотрена возможность записи выходных данных\n";
		return;
	} else
		manager_->set_task_out(nameAlg, nameFile, num);
	return;
}

void Mad::get_count_queue(const std::string& name) {
	if (name == "sink")
		std::cout
				<< "В выходной очереди потока приёмника данных от АЦП находится "
				<< (sinkAdc_->get_count_queue()) << " элементов\n";
	else {
		int num = manager_->get_count_queue(name);
		if (num != -1)
			std::cout << "во входной очереди " << " алгоритма " << name
					<< " находится " << num << " элементов\n";
	}
	return;
}

Mad::Mad(const int& sock, void (*pf)(void*, size_t, int), ManagerAlg *m,
		SinkAdcData *s) :
		sock_(sock), isRunThreadAdc_(false), isRunThreadTest_(false), pass_(pf), manager_(
				m), sinkAdc_(s) {
//открытие файлов связи с акустической платой
	int addr = 3;
	if ((f3_i2c_ = open(devI2C_, O_RDWR)) < 0) {
		std::cerr << "Невозможно получить доступ к шине i2c\n";
		exit(1);
	}
	if (ioctl(f3_i2c_, I2C_SLAVE, addr) < 0) {
		std::cerr
				<< "Невозможно связаться с i2c устройством по данному адресу.\n";
		exit(1);
	}
	f3_spi_ = open(devSPI_, O_RDWR);
	if (f3_spi_ == -1) {
		std::cerr << "Невозможно открыть файл устройств f3\n";
		exit(1);
	}
	sinkAdc_->set_freq(FREQ_);
	short gain[4] = { GAIN_, GAIN_, GAIN_, GAIN_ };
	sinkAdc_->set_gain(gain);
	//добавление алгортмов в менеджер
	algCont_ = new ContinueAlg(name_alg_[CONTINIOUS], CONTINIOUS, pass_);
	manager_->addAlgorithm(algCont_);
}

Mad::~Mad() {
	close(f3_i2c_);
	close(f3_spi_);
	manager_->turnOffAll();
	delete algCont_;
	return;
}

bool Mad::start_adc(void) {
	bool status = false;
	if (isRunThreadAdc_) {
		std::cout << "Процесс ацп преобразования уже запущен\n";
		status = true;
		return status;
	} else if (isRunThreadTest_) {
		std::cout << "Процесс тестового преобразования уже запущен\n";
		return status;
	}
	char buf = STOP_ADC;
	if (write(f3_i2c_, &buf, 1) != 1) {
		std::cout
				<< "Приказ об остановке АЦП преобразования передать не удалось\n";
		return status;
	}
	usleep(1000);
	if (ioctl(f3_spi_, IOCTL_ADC_START) < 0) {
		std::cout
				<< "Не удалось передать команду IOCTL_ADC_START в драйвер f3_spi\n";
		return status;
	}
	buf = START_ADC;
	if (write(f3_i2c_, &buf, 1) == 1) {
		std::cout << "Передан приказ о запуске АЦП преобразования\n";
		isRunThreadAdc_ = true;
		status = true;
	} else
		std::cout
				<< "Приказ о запуске АЦП преобразования передать не удалось\n";
	return status;
}
} /* namespace mad_n */
