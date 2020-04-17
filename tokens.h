/*
 * tokens.h
 *
 *  Created on: Mar. 14, 2020
 *      Author: solomon
 */

#ifndef TOKENS_H_
#define TOKENS_H_

#include <iostream>
#include <string>
#include <map>

extern bool STARTED;

extern std::map <std::string, std::string> KEY_TOKENS;
extern std::map <std::string, std::string> TOKEN_NEEDS_SPACE;

int startup();
int tokenize(std::istream& in, std::ostream& out, std::vector<std::pair<std::string, std::string>>& token_vector);

#endif /* TOKENS_H_ */
