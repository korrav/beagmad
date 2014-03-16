/*
 * gpio_overload.h
 *
 *  Created on: 16 марта 2014 г.
 *      Author: andrej
 */

#ifndef GPIO_OVERLOAD_H_
#define GPIO_OVERLOAD_H_
int open_file_gpio_overload(void); /*инициализирует gpio, сигнализирующее о перегрузке в PGA и
 возвращает дескриптор открытого файла, иначе отрицательное число*/

#endif /* GPIO_OVERLOAD_H_ */
