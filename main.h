/*
 * main.h
 *
 *  Created on: 15 февр. 2014 г.
 *      Author: andrej
 */

#ifndef MAIN_H_
#define MAIN_H_

#define BAG_ADDR	"192.168.203.35"
#define BAG_PORT	30000
#define MAD_PORT	30000
#define MAX_SIZE_SAMPL_SEND 4100 //определяет длину буфера передачи сокета
#define SIZE_REC_BUF (MAX_SIZE_SAMPL_SEND * 2) //определяет длину приёмного буфера
#define DEV_I2C	"/dev/i2c-1"
#define DEV_SPI "/dev/f3_spi"
#define FILE_CONFIG_MAD	"./config"	//конфигурационный файл МАДа, заданный по умолчанию

#endif /* MAIN_H_ */
