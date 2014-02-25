/*
 * WriteDataToFile.cpp
 *
 *  Created on: 21 февр. 2014 г.
 *      Author: andrej
 */

#include "WriteDataToFile.h"
#include <iostream>
#include <fcntl.h>

namespace mad_n {

WriteDataToFile::WriteDataToFile() :
		isWrite_(false), numSampl_(0), count_(0) {

}

bool WriteDataToFile::set_task(const std::string& nameFile, const int& num) {
	nameFile_ = nameFile;
	file_.open(nameFile, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!file_.is_open()) {
		std::cout << "Нет возможности создать файл " << nameFile << std::endl;
		return false;
	} else {
		numSampl_ = count_ = num;
		nameFile_ = nameFile;
		isWrite_ = true;
	}
	return true;
}

void WriteDataToFile::closeWriteFile(void) {
	isWrite_ = false;
	numSampl_ = 0;
	count_ = 0;
	file_.close();
	return;
}

void WriteDataToFile::write(void* pbuf, const size_t& size) {
	if (isWrite_) {
		if (numSampl_ == SIZE_P) {
			file_.write(reinterpret_cast<char*>(pbuf), size);
			std::cout << "В файл " << nameFile_ << " записано " << size
					<< " байт\n";
			closeWriteFile();
		} else {
			int num;
			if (count_ - size < 0)
				num = count_;
			else
				num = size;
			file_.write(reinterpret_cast<char*>(pbuf), num);
			if ((count_ -= num) <= 0) {
				std::cout << "В файл " << nameFile_ << " записано " << numSampl_
						<< " отсчётов\n";
				closeWriteFile();
			}

		}
	}
	return;
}

WriteDataToFile::~WriteDataToFile() {
	file_.close();
	return;
}

} /* namespace mad_n */
