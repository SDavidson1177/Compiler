/*
 * tokens.h
 *
 *  Created on: Apr. 30, 2020
 *      Author: solomon
 */

#ifndef ASMBUFFER_H_
#define ASMBUFFER_H_

#include <vector>
#include <fstream>
#include <string>

class AsmBuffer{
	std::vector<std::string> vec;
	std::ofstream& out;
	unsigned capacity;
public:

	AsmBuffer(std::ofstream& out, unsigned cap = 100);
	~AsmBuffer();

	void emplace_back(std::string&& arg);
	std::vector<std::string>::iterator begin();
	std::vector<std::string>::iterator end();

};

#endif
