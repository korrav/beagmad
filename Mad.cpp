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
#include <fstream>
#include <stdexcept>
#include <bitset>
#include <functional>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

enum i2c_id {
	START_ADC,
	STOP_ADC,
	SET_GAIN,
	START_SPI0,
	START_SPI1,
	START_TEST,
	STOP_TEST,
	CLEAR_SET_OVERLOAD,
	SET_PERIOD_MONITORING
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
Head Mad::config_;
bool Mad::sync(void) {
	bool status = true;
	if (ioctl(f3_spi_, IOCTL_ADC_SYNC) < 0) {
		std::cout
		<< "Не удалось передать команду IOCTL_ADC_START в драйвер f3_spi\n";
		status = false;
	}
	return status;
}

void Mad::clear_set_overload(void) {
	char buf = CLEAR_SET_OVERLOAD;
	if (write(f3_i2c_, &buf, 1) != 1)
		std::cout
		<< "Приказ о подтверждении приёма сообщения о перегрузке pga передать не удалось\n";
	return;
}
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
			buf[i] = 0;
		else if (gain[i - 1] >= GAIN_ + 9 && gain[i - 1] <= GAIN_ + 60) {
			N = (gain[i - 1] - GAIN_ - 6) / 3;
			gain[i - 1] = GAIN_ + 6 + 3 * N;
			buf[i] = N;
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

void Mad::receive(PtrData pPack) {
	PtrData pBuf;
	int idBlock = -1;
	std::make_pair(std::ref(pBuf), std::ref(idBlock)) = col_.receivBlock(pPack);
	if (pBuf == nullptr || pBuf->size() < sizeof(int))
		return;
	if(idBlock != COMMAND)
		return;
	unsigned len_com = pBuf->size();
	int* pCommand = reinterpret_cast<int*>(pBuf->data());
	int* arg = pCommand + 1;
	h_pack_ans answer = {*pCommand};
	switch (*pCommand) {
	case ID_START_ADC:
		if (len_com != sizeof(int))
			break;
		if (start_adc())
			answer.status = OK;
		else
			answer.status = NOT_OK;
		passAnswer(&answer, sizeof(answer));
		break;
	case ID_STOP_ADC:
		if (len_com != sizeof(int))
			break;
		if (stop_adc())
			answer.status = OK;
		else
			answer.status = NOT_OK;
		passAnswer(&answer, sizeof(answer));
		break;
	case ID_START_TEST:
		if (len_com != sizeof(int))
			break;
		if (start_test())
			answer.status = OK;
		else
			answer.status = NOT_OK;
		passAnswer(&answer, sizeof(answer));
		break;
	case ID_STOP_TEST:
		if (len_com != sizeof(int))
			break;
		if (stop_test())
			answer.status = OK;
		else
			answer.status = NOT_OK;
		passAnswer(&answer, sizeof(answer));
		break;
	case ID_SET_GAIN:
		if (len_com != 5 * sizeof(int))
			break;
		if (set_gain(arg))
			answer.status = OK;
		else
			answer.status = NOT_OK;
		passAnswer(&answer, sizeof(answer));
		break;
	case OPEN_ALG:
		if (len_com != 2 * sizeof(int))
			break;
		if (open_alg(*arg))
			answer.status = OK;
		else
			answer.status = NOT_OK;
		passAnswer(&answer, sizeof(answer));
		break;
	case CLOSE_ALG:
		if (len_com != 2 * sizeof(int))
			break;
		if (close_alg(*arg))
			answer.status = OK;
		else
			answer.status = NOT_OK;
		passAnswer(&answer, sizeof(answer));
		break;
	case CLOSE_ALL_ALG:
		if (len_com != sizeof(int))
			break;
		close_all_alg();
		answer.status = OK;
		passAnswer(&answer, sizeof(answer));
		break;
	case SET_SIGMA:
		if (len_com != 2 * sizeof(int))
			break;
		algExN_->set_sigma(*arg);
		answer.status = OK;
		passAnswer(&answer, sizeof(answer));
		break;
	case SET_PERIOD_MONITOR_PGA:
		if (len_com != 2 * sizeof(int))
			break;
		if (set_period_monitoring_pga(static_cast<unsigned char> (*arg)))
			answer.status = OK;
		else
			answer.status = NOT_OK;
		passAnswer(&answer, sizeof(answer));
		break;
	case SET_PB:
		if (len_com != 3 * sizeof(int))
			break;
		if (algExN_->set_parameter_block(arg[0], arg[1]))
			answer.status = OK;
		else
			answer.status = NOT_OK;
		passAnswer(&answer, sizeof(answer));
		break;

	case GET_PB: {
		if (len_com != sizeof(int))
			break;
		char *compl_answer = new char[sizeof(answer) + 2 * sizeof(int)];
		reinterpret_cast<h_pack_ans*>(compl_answer)->id = answer.id;
		unsigned int* param = reinterpret_cast<unsigned int*>(compl_answer
				+ sizeof(h_pack_ans));
		if (algExN_->get_parameter_block(param, param + 1))
			reinterpret_cast<h_pack_ans*>(compl_answer)->status = OK;
		else
			reinterpret_cast<h_pack_ans*>(compl_answer)->status = NOT_OK;
		passAnswer(compl_answer, sizeof(answer) + 2 * sizeof(int));
		delete[] compl_answer;
	}
	break;
	case GET_AC: {
		if (len_com != sizeof(int))
			break;
		std::list<int> l;
		get_list_active_algoritm(&l);
		char *compl_answer = new char[sizeof(answer) + l.size() * sizeof(int)];
		reinterpret_cast<h_pack_ans*>(compl_answer)->id = answer.id;
		reinterpret_cast<h_pack_ans*>(compl_answer)->status = OK;
		int* param = reinterpret_cast<int*>(compl_answer + sizeof(h_pack_ans));
		auto pl = l.begin();
		for (unsigned int i = 0; i < l.size(); i++, pl++)
			param[i] = *pl;

		passAnswer(compl_answer, sizeof(answer) + l.size() * sizeof(int));
	}
	break;
	case SYNC:
		if (len_com != sizeof(int))
			break;
		if (sync())
			answer.status = OK;
		else
			answer.status = NOT_OK;
		passAnswer(&answer, sizeof(answer));
		break;
	case SET_PERIOD_MONITOR:
		if (len_com != 2 * sizeof(int))
			break;
		set_period_monitor(arg[0]);
		answer.status = OK;
		passAnswer(&answer, sizeof(answer));
		break;
	case GET_PERIOD_MONITOR: {
		if (len_com != sizeof(int))
			break;
		char *compl_answer = new char[sizeof(answer) + sizeof(unsigned int)];
		reinterpret_cast<h_pack_ans*>(compl_answer)->id = answer.id;
		reinterpret_cast<h_pack_ans*>(compl_answer)->status = OK;
		unsigned int* param = reinterpret_cast<unsigned int*>(compl_answer
				+ sizeof(h_pack_ans));
		*param = get_period_monitor();
		passAnswer(compl_answer, sizeof(answer) + sizeof(unsigned int));
	}
	break;
	case GET_SIGMA: {
		if (len_com != sizeof(int))
			break;
		char *compl_answer = new char[sizeof(answer) + sizeof(int)];
		reinterpret_cast<h_pack_ans*>(compl_answer)->id = answer.id;
		reinterpret_cast<h_pack_ans*>(compl_answer)->status = OK;
		int* param = reinterpret_cast<int*>(compl_answer + sizeof(h_pack_ans));
		*param = algExN_->get_sigma();
		passAnswer(compl_answer, sizeof(answer) + sizeof(int));
	}
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

void Mad::get_list_active_algoritm(std::list<std::string>* la) {
	manager_->get_active_algorithm(la, nullptr);
	return;
}

void Mad::get_list_active_algoritm(std::list<int>* la) {
	manager_->get_active_algorithm(nullptr, la);
	return;
}

void Mad::post_overload(void) {
	int buf[4] = { 34, 34, 34, 34 };
	std::cout
	<< "Произошла перегрузка питания усилителей PGA. Выполняется принудительное обнуление коэффициентов усиления\n";
	set_gain(buf);
	clear_set_overload();
	int buf_tr = OVERLOAD;
	passInfo(&buf_tr, sizeof(buf_tr));
	return;
}

Mad::Mad(void (*pf)(std::vector<int8_t>& pbuf, int id_block), ManagerAlg *m, SinkAdcData *s, std::string nameFileConfig) :
		isRunThreadAdc_(false), isRunThreadTest_(false), pass__(pf), manager_(m), sinkAdc_(s) {
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
	//добавление алгоритмов в менеджер
	algCont_ = new ContinueAlg(name_alg_[CONTINIOUS], CONTINIOUS, pf);
	manager_->addAlgorithm(algCont_);
	algExN_ = new ExcessNoiseAlg(name_alg_[GASIK], GASIK, pf, manager_);
	manager_->addAlgorithm(algExN_);
	algFil_ = new DetectionOfNeutrinos(name_alg_[FILTER], FILTER, pf);
	return;
}

bool Mad::set_period_monitoring_pga(unsigned char period) {
	bool status = false;
	char buf[2] = { SET_PERIOD_MONITORING, static_cast<char>(period) };
	if (write(f3_i2c_, &buf, 2) == 2) {
		std::string message;
		message = (period == 0 ? "Мониторинг деятельности pga запрещён" :
				std::string("Установлен перод монитора pga ") + std::to_string(period));
		std::cout << message << std::endl;
		status = true;
	} else
		std::cout
		<< "Попытка установить новый период мониторинга pga потерпела неудачу\n";
	return status;
}

void Mad::fillConfig(std::string file) {
	if(!boost::filesystem::exists(file)) {
		std::cerr << "Конфигурационного файла" << file << " не существует\n";
		exit(1);
	}
	std::ifstream configFile(file);
	if(!configFile) {
		std::cerr << "Невозможно открыть конфигурационный файл\n";
		exit(1);
	}
	std::string message;
	std::vector<std::string> vecMes;
	std::bitset<8> stateUsedArg;
	using namespace boost::algorithm;
	while(std::getline(configFile, message)) {
		vecMes.clear();
		trim(message);
		split(vecMes, message, is_space(), token_compress_on);
		try {
			if(to_upper_copy(vecMes[0]) == "FORMAT_VER" && vecMes.size() >= 2) {
				config_.verSoft = std::stoi(vecMes[1]);
				stateUsedArg.set(0);
			} else if(to_upper_copy(vecMes[0]) == "MAD" && vecMes.size() >= 2) {
				config_.numMad = std::stoi(vecMes[1]);
				stateUsedArg.set(1);
			} else if(to_upper_copy(vecMes[0]) == "HARD_VER" && vecMes.size() >= 2) {
				config_.verHard = std::stoi(vecMes[1]);
				stateUsedArg.set(2);
			} else if(to_upper_copy(vecMes[0]) == "FREQ" && vecMes.size() >= 2) {
				config_.freq = std::stoi(vecMes[1]);
				stateUsedArg.set(3);
			} else if(to_upper_copy(vecMes[0]) == "GAIN" && vecMes.size() >= 5){
				for(int i = 0; i < 4; i++)
					config_.gain[i] = std::stoi(vecMes[1 + i]);
				stateUsedArg.set(4);
			} else if(to_upper_copy(vecMes[0]) == "COORD_HYD" && vecMes.size() >= 13) {
				for(int hyd = 0; hyd < 4; hyd++)
					for(int i = 0; i < 3; i++)
						config_.coordHyd[hyd][i] = std::stoi(vecMes[1 + 3 * hyd + i]);
				stateUsedArg.set(5);
			} else if(to_upper_copy(vecMes[0]) == "AFC" && vecMes.size() >= 13) {
				for(int hyd = 0; hyd < 4; hyd++)
					for(int i = 0; i < 3; i++)
						config_.afc[hyd][i] = std::stoi(vecMes[1 + 3 * hyd + i]);
				stateUsedArg.set(6);
			} else if(to_upper_copy(vecMes[0]) == "NUMB_HYD" && vecMes.size() >= 5){
				for(int i = 0; i < 4; i++)
					config_.numHyd[i] = std::stoi(vecMes[1 + i]);
				stateUsedArg.set(7);
			}
		} catch (const std::invalid_argument& ia) {
			std::cerr << "Неправильно отформатирован конфигурационный файл. Аргумент содержит нечисловое значение\n";
			exit(1);
		}
		catch (const std::out_of_range& oor) {
			std::cerr << "Неправильно отформатирован конфигурационный файл. Аргумент содержит слишком большое значение\n";
			exit(1);
		}
	}
	if(!stateUsedArg.all()) {
		std::cerr << "Неправильно отформатирован конфигурационный файл. Учтены не все параметры конфигурации\n";
		exit(1);
	}
}

boost::optional<int> Mad::getMadIdFromConfigFile(const std::string& file) {
	if(!boost::filesystem::exists(file)) {
		std::cerr << "Конфигурационного файла" << file << " не существует\n";
		exit(1);
	}
	std::ifstream configFile(file);
	if(!configFile) {
		std::cerr << "Невозможно открыть конфигурационный файл\n";
		exit(1);
	}
	std::string message;
	std::vector<std::string> vecMes;
	using namespace boost::algorithm;
	while(std::getline(configFile, message)) {
		vecMes.clear();
		trim(message);
		split(vecMes, message, is_space(), token_compress_on);
		try {
			if(to_upper_copy(vecMes[0]) == "MAD" && vecMes.size() >= 2)
				return std::stoi(vecMes[1]);
		}
		catch (const std::invalid_argument& ia) {
			std::cerr << "Неправильно отформатирован конфигурационный файл. Аргумент содержит нечисловое значение\n";
			exit(1);
		}
		catch (const std::out_of_range& oor) {
			std::cerr << "Неправильно отформатирован конфигурационный файл. Аргумент содержит слишком большое значение\n";
			exit(1);
		}
	}
	return boost::optional<int>{};
}

Head Mad::getConfig(void) {
	return config_;
}

void Mad::passAnswer(void* pbuf, size_t size) {
	Head config = getConfig();
	sinkAdc_->get_gain(config.gain);
	config.freq = sinkAdc_->get_freq();
	size_t sizeFull = sizeof(Head) + size;
	std::vector<int8_t> pBufFull(sizeFull);
	auto iterator = std::copy_n(reinterpret_cast<int8_t*>(&config), sizeof(config), pBufFull.begin());
	std::copy_n(reinterpret_cast<int8_t*>(pbuf), sizeof(size), iterator);
	pass__(pBufFull, ANSWER);
	return;
}

void Mad::passInfo(void* pbuf, size_t size) {
	Head config = getConfig();
	sinkAdc_->get_gain(config.gain);
	config.freq = sinkAdc_->get_freq();
	size_t sizeFull = sizeof(Head) + size;
	std::vector<int8_t> pBufFull(sizeFull);
	auto iterator = std::copy_n(reinterpret_cast<int8_t*>(&config), sizeof(config), pBufFull.begin());
	std::copy_n(reinterpret_cast<int8_t*>(pbuf), sizeof(size), iterator);
	pass__(pBufFull, INFO);
	return;
}

Mad::~Mad() {
	close(f3_i2c_);
	close(f3_spi_);
	manager_->turnOffAll();
	delete algCont_;
	delete algExN_;
	delete algFil_;
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

void Mad::set_period_monitor(const unsigned& s) {
	manager_->monitor_.set_period(s);
	return;
}

unsigned int Mad::get_period_monitor(void) {
	return manager_->monitor_.get_period();
}

} /* namespace mad_n */
