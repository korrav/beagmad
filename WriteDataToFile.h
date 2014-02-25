/*
 * WriteDataToFile.h
 *
 *  Created on: 21 февр. 2014 г.
 *      Author: andrej
 */

#ifndef WRITEDATATOFILE_H_
#define WRITEDATATOFILE_H_
#include <string>
#include <fstream>

namespace mad_n {

/*
 *
 */
class WriteDataToFile {
	bool isWrite_; //разрешение произвести запись в файл
	int numSampl_;	//сколько необходимо записать в файл отсчётов; -1
	int count_;		//сколько ещё осталось записать
	std::ofstream file_; //выходной поток, куда записываются данные
	std::string nameFile_; //имя файла
public:
	static const int SIZE_P = -1; //предусматривает только единичную запись в файл
	bool set_task(const std::string& nameFile, const int& num = SIZE_P); //установить задание на запись
	void closeWriteFile(void); //завершение записи данных в файл
	void write(void* pbuf, const size_t& size); //записать данные в файл
	WriteDataToFile();
	virtual ~WriteDataToFile();
};

} /* namespace mad_n */

#endif /* WRITEDATATOFILE_H_ */
