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

const unsigned int NUM_SAMPL_PACK = 2000; //количество отсчётов, который может поместиться в один пакет данных
enum id { //идентификаторы блоков данных
	COMMAND,	//блок, содержащий команду
	ANSWER, //блок, содержащий ответ на команду
	FILTER_ALG,	//блок данных в режиме фильтрации
	CONTINIOUS_ALG, //блок данных в режиме непрерывной передачи
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

#endif /* WRAPING_H_ */
