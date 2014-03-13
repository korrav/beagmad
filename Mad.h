/*
 * Mad.h
 *
 *  Created on: 11 февр. 2014 г.
 *      Author: andrej
 */

#ifndef MAD_H_
#define MAD_H_
#include "ManagerAlg.h"
#include "SinkAdcData.h"
#include "ContinueAlg.h"
#include "ExcessNoiseAlg.h"
#include "main.h"
#include <string>
#include <list>
namespace mad_n {
/*
 *
 */

class Mad {
	enum id_command {
		ID_START_ADC, //запустить АЦП преобразование
		ID_STOP_ADC, //остановить АЦП преобразование
		ID_SET_GAIN, //установить коэффициенты усиления
		ID_START_TEST, //начать тестирование драйвера АЦП
		ID_STOP_TEST, //завершить тестирование драйвера АЦП
		OPEN_ALG, //запустить алгоритм
		CLOSE_ALG, //остановить алгоритм
		CLOSE_ALL_ALG, //закрыть все действующие алгоритмы
		GET_AC, //получить информацию о действующих в данный момент алгоритмах
		SYNC, //синхронизация АЦП
		SET_PERIOD_MONITOR, //установить период передачи мониторограмм
		GET_PERIOD_MONITOR, //получить период передачи мониторограмм
		/*специализированные команды*/
		//алгоритм gas
		SET_SIGMA, //установить коэффициент превышения шума
		SET_PB, //установить параметры блока данных, передаваемого на БЦ
		GET_PB, //получить параметры блока данных, передаваемого на БЦ
	};
	enum status_ans {
		NOT_OK, OK
	};
	enum id_alg {
		CONTINIOUS = 0, GASIK
	};
	const std::string name_alg_[2] = { "cont", "gas" };
	int sock_; //ethernet socket
	int f3_i2c_; //дескриптор файла управления акустической платой
	int f3_spi_; //дескриптор файла приёма данных от акустической платы
	bool isRunThreadAdc_;	//поток обработки данных АЦП запущен
	bool isRunThreadTest_;	//поток обработки тестовых данных запущен
	const char* devI2C_ = DEV_I2C; //файл устройства I2C2
	const char* devSPI_ = DEV_SPI; //файл устройства SPI
	const int FREQ_ = 528000; //частота дискретизации (в Гц)
	const short GAIN_ = 34; //начальный коэффициент усиления для каналов (дцБ)
	void (*pass_)(void* pbuf, size_t size, int id_block); //функция передачи данных
	ManagerAlg *manager_;	//менеджер алгоритмов
	SinkAdcData *sinkAdc_;	//приёмник данных от АЦП
public:
	void set_period_monitor(const unsigned& s); //установить период передачи мониторограмм
	unsigned int get_period_monitor(void); //получить период передачи мониторограмм
	void receive(const unsigned int& len, void* pbuf); //обработка принятых по Ethernet данных
	bool sync(void); //отправка синхросигнала в драйвер АЦП
	bool start_adc(void); //запуск АЦП преобразования
	bool stop_adc(void); //остановка АЦП преобразования
	bool start_test(void); //запуск тестового преобразования
	bool stop_test(void); //остановка тестового преобразования
	bool set_gain(int* gain); //установка коэффициентов усиления
	bool start_spi0(void); //запуск единичной передачи spi0
	bool start_spi1(void); //запуск единичной передачи spi1
	void read_reg(void); //запрос дампа памяти модуля f3_spi
	void open_alg(const std::string& name);	//включить алгоритм
	bool open_alg(const int& id);	//включить алгоритм
	void close_alg(const std::string& name);	//выключить алгоритм
	bool close_alg(const int& id);	//выключить алгоритм
	void close_all_alg(void); //выключить все алгортмы
	void get_count_queue(const std::string& name); //возвратить количество элементов в очереди
	void set_task_in(const std::string nameAlg, const std::string& nameFile,
			const int& num = WriteDataToFile::SIZE_P); //установить задание на запись данных входной очереди алгоритма
	void set_task_out(const std::string nameAlg, const std::string& nameFile,
			const int& num = WriteDataToFile::SIZE_P); //установить задание на запись данных выходной очереди алгоритма
	void get_list_active_algoritm(std::list<std::string> *la); //получить список активных алгоритмов
	void get_list_active_algoritm(std::list<int> *la); //получить список активных алгоритмов
	Mad(const int& sock, void (*pf)(void*, size_t, int), ManagerAlg *m,
			SinkAdcData *s);
	Mad(const Mad&) = delete;
	Mad& operator =(const Mad&) = delete;
	virtual ~Mad();
//алгоритмы
	ContinueAlg *algCont_; //алгоритм непрерывной передачи данных
	ExcessNoiseAlg *algExN_; //алгоритм по превышению уровня шумов
};

} /* namespace mad_n */

#endif /* MAD_H_ */
