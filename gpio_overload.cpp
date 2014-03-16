#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include "gpio_overload.h"
using namespace std;

static string NameDirGpio = "/sys/class/gpio"; //директория в пространстве sys устройств gpio
static const unsigned int GpioOverload = 46;

static bool gpio_export(unsigned int gpio) { //функция для экспорта конкретного вывода gpio в пространство sys
	int fd = open((NameDirGpio + "/export").c_str(), O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return false;
	}
	string num_gpio = to_string(gpio);
	write(fd, num_gpio.c_str(), num_gpio.length() + 1);
	close(fd);
	return true;
}

static bool gpio_set_dir(unsigned int gpio) { //функция, устанавливающая контакт в режим ввода
	size_t len;
	int fd = open(
			(NameDirGpio + "/gpio" + to_string(gpio) + "/direction").c_str(),
			O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return false;
	}
	len = write(fd, "in", 3);
	close(fd);
	if(len != 3) {
		perror("fail write gpio/direction");
		return false;
	}
	return true;
}

static bool gpio_set_edge(unsigned int gpio) { //функция, устанавливающая рекцию на прерывание по переднему фронту
	size_t len;
	int fd = open((NameDirGpio + "/gpio" + to_string(gpio) + "/edge").c_str(),
	O_WRONLY);
	if (fd < 0) {
		perror("gpio/edge");
		return false;
	}
	len = write(fd, "rising", 7);
	close(fd);
	if(len != 7) {
		perror("fail write gpio/rising");
		return false;
	}
	return true;
}

int open_file_gpio_overload(void) { //инициализирует gpio, сигнализирующее о перегрузке в PGA
	int fd = -1;
	if (!gpio_export(GpioOverload))
		return fd;
	if (!gpio_set_dir(GpioOverload))
		return fd;
	if (!gpio_set_edge(GpioOverload))
		return fd;
	fd =
			open(
					(NameDirGpio + "/gpio" + to_string(GpioOverload) + "/value").c_str(),
					O_RDONLY);
	if (fd < 0)
		perror("gpio/edge");
	return fd;
}
