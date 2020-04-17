/*
 * errorcodes.h
 *
 *  Created on: Mar. 14, 2020
 *      Author: solomon
 */

#ifndef ERRORCODES_H_
#define ERRORCODES_H_

#include <string>

typedef int err_code;

struct thrown_e{
	err_code e;
	std::string message;

	thrown_e(err_code e_p, std::string message_p):e(e_p), message(message_p){

	}
};

const int OK = 0;
const int TOKEN_ERROR = -1;
const int PARSE_ERROR = -2;
const int CONTEXT_ERROR = -3;

#endif /* ERRORCODES_H_ */
