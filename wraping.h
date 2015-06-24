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


const unsigned int NUM_SAMPL_PACK = 2000; //количество отсчётов, который может поместиться в один пакет данных
enum id { //идентификаторы блоков данных
	COMMAND,	//блок, содержащий команду
	ANSWER, //блок, содержащий ответ на команду
	FILTER_ALG,	//блок данных в режиме фильтрации
	CONTINIOUS_ALG, //блок данных в режиме непрерывной передачи
	MONITOR, //блок мониторограммы
	FILTER_NOISE, //блок данных, фильтрованный по превышению уровня шума
	INFO, //блок сообщений от МАД
};

struct h_package { //структура, в которую оборачивается все пакеты данных, принимаемых, либо отправляемых МАД
	int idSrc;	//идентификатор источника
	int idBlock;	//идентификатор блока
};

struct h_pack_com { //структура, содержащая командный блок
	h_package head; //шапка
	int id; //идентификатор команды
};

struct h_pack_ans { //структура, содержащая ответ на команду
	int id; //идентификатор команды
	int status; //результат выполнения команды (OK или NOT_OK)
};

struct datA { //структура данных, в которую оборачиваются результаты алгоритмов по поиску нейтрино
	short gain[4];	//коэффициенты усиления каналов (в дЦб)
	int freq;	//частота дискретизации (в Гц)
	unsigned int numFirstCount; //номер первого отсчёта
	int amountCount; //количество отсчётов (1 отс = 4 x 2 байт)
	short sampl[NUM_SAMPL_PACK * 4]; //отсчёты
};

#define LAST_PACKAGE -1
struct datAfilterNoise { //структура данных, в которую оборачиваются результаты алгоритма FILTER_NOISE
	int num; //порядковый номер пакета в рвмках единичного блока данных, последний - LAST
	short gain[4];	//коэффициенты усиления каналов (в дЦб)
	int freq;	//частота дискретизации (в Гц)
	unsigned int numFirstCount; //номер первого отсчёта
	int amountCount; //количество отсчётов (1 отс = 4 x 2 байт)
	short sampl[NUM_SAMPL_PACK * 4]; //отсчёты
};

struct monitorogramm { //структура данных, в которую оборачиваются результаты вычисления статистики
	short gain[4];	//коэффициенты усиления каналов (в дЦб)
	int freq;	//частота дискретизации (в Гц)
	int rms[4]; 	//СКО
	int mean[4];	//математическое ожидание
};

#endif /* WRAPING_H_ */
