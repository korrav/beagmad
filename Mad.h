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
#include <boost/optional.hpp>
#include "Collector.h"
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
		GET_SIGMA, //получить текущее занчение коэффициента превышение шума
		SET_PB, //установить параметры блока данных, передаваемого на БЦ
		GET_PB, //получить параметры блока данных, передаваемого на БЦ
		SET_PERIOD_MONITOR_PGA,	//установить новый период мониторинга pga
	};

	enum id_info {
		OVERLOAD //сообщение о перегрузке каналов МАД
	};

	enum status_ans {
		NOT_OK, OK
	};

	const std::string name_alg_[2] = { "cont", "gas" };
	int f3_i2c_; //дескриптор файла управления акустической платой
	int f3_spi_; //дескриптор файла приёма данных от акустической платы
	bool isRunThreadAdc_;	//поток обработки данных АЦП запущен
	bool isRunThreadTest_;	//поток обработки тестовых данных запущен
	const char* devI2C_ = DEV_I2C; //файл устройства I2C2
	const char* devSPI_ = DEV_SPI; //файл устройства SPI
	const int FREQ_ = 528000; //частота дискретизации (в Гц)
	const short GAIN_ = 34; //начальный коэффициент усиления для каналов (дцБ)
	void (*pass__)(std::vector<int8_t>& pbuf, int id_block); //функция передачи данных
	ManagerAlg *manager_;	//менеджер алгоритмов
	SinkAdcData *sinkAdc_;	//приёмник данных от АЦП
	static Head config_;	//данные о конфигурации МАД
	collector_n::Collector col_;	//коллектор пакетов данных
	void fillConfig(std::string file);	//заполнение конфигурационных данных МАД
	void clear_set_overload(void); //подтверждение приёма сообщения о перегрузке pga
	void passAnswer(void* pbuf, size_t size); //функция передачи ответа на команду
	void passInfo(void* pbuf, size_t size); //функция передачи информационного сообщения
public:
	static Head getConfig(void);	//возвратить структуру конфигурации МАД
	static boost::optional<int> getMadIdFromConfigFile(const std::string& file);	//получить идентификатор МАД из конфигурационного файла
	void post_overload(void); //сообщение МАД о перегрузке pga
	void set_period_monitor(const unsigned& s); //установить период передачи мониторограмм
	unsigned int get_period_monitor(void); //получить период передачи мониторограмм
	void receive(PtrData pPack); //обработка принятых по Ethernet данных
	bool sync(void); //отправка синхросигнала в драйвер АЦП
	bool start_adc(void); //запуск АЦП преобразования
	bool stop_adc(void); //остановка АЦП преобразования
	bool start_test(void); //запуск тестового преобразования
	bool stop_test(void); //остановка тестового преобразования
	bool set_gain(int* gain); //установка коэффициентов усиления
	bool set_period_monitoring_pga(unsigned char period);	//установка периода мониторинга деятельности pga; 0 - мониторинг отключен
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
	Mad(void (*pf)(std::vector<int8_t>& pbuf, int id_block), ManagerAlg *m, SinkAdcData *s, std::string nameFileConfig);
	Mad(const Mad&) = delete;
	Mad& operator =(const Mad&) = delete;
	virtual ~Mad();
//алгоритмы
	ContinueAlg *algCont_; //алгоритм непрерывной передачи данных
	ExcessNoiseAlg *algExN_; //алгоритм по превышению уровня шумов
};

} /* namespace mad_n */

#endif /* MAD_H_ */
