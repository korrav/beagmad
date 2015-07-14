/*
 * wraping.h
 *
 *  Created on: 02 февр. 2014 г.
 *      Author: andrej
 */
/*Представлены функции,которые формируют пакеты блоки данных, предназначенных для передачи на берег*/
#ifndef WRAPING_H_
#define WRAPING_H_
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory>

typedef std::shared_ptr<std::vector<int8_t>> PtrData;

struct MarkPack {	//метка пакета
	int id;	//уникальный номер блока данных
	int part;	//порядковый номер куска блока данных
	int totalPart;	//общее количество частей в пакете
};

struct SrcPack {
	int idSrc;	//идентификатор источника
	int idBlock;	//идентификатор блока
};

struct HeadPack {	//заголовок пакета
	SrcPack endpoints;
	MarkPack mark;
};

#define NUM_PARAM_HYD 3
#define NUM_PARAM_AFC 3

struct Head {
	int32_t verSoft;	//версия формата пакета и файла метаданных
	int32_t numMad;	//номер МАДа
	int32_t verHard;	//версия аппаратуры
	int32_t freq;	//частота дискретизации
	int32_t gain[4];	//коэффициенты усиления в каналах
	int32_t coordHyd[4][NUM_PARAM_HYD];	//координаты гидрофонов МАДа
	int32_t afc[4][NUM_PARAM_AFC];	//АЧХ
	int32_t numHyd[4];	//номера гидрофонов
	int32_t numAlg;	//номер Алгоритма
	int32_t verAlg;	//версия алгоритма
};

enum id_block {	//идентификатор типа пакета
	DATA,
	COMMAND,	//блок, содержащий команду
	ANSWER,	//блок, содержащий ответ на команду
	INFO	//блок сообщений от МАД
};

enum id_data {	//идентификаторы пакетов данных
	MONITOR,	//блок мониторограммы
	FILTER,
	GASIK,
	CONTINIOUS
};

struct DataAlgorithm { //структура данных, в которую оборачиваются результаты алгоритмов по поиску нейтрино
	unsigned int numFirstCount; //номер первого отсчёта
	int8_t data; //первый байт данных
};

struct Monitor { //структура данных, в которую оборачиваются результаты вычисления статистики
	int rms[4]; 	//СКО
	int mean[4];	//математическое ожидание
};


struct GasikParams {
	unsigned int level;
};

struct Gasik {
	GasikParams param;
	DataAlgorithm buf;
};

struct h_pack_ans { //структура, содержащая ответ на команду
	int id; //идентификатор команды
	int status; //результат выполнения команды (OK или NOT_OK)
};

#endif /* WRAPING_H_ */
