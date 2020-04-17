/*
 * symbol.h
 *
 *  Created on: Apr. 7, 2020
 *      Author: solomon
 */

#ifndef SYMBOL_H_
#define SYMBOL_H_

#include <string>
#include "grammars.h"

typedef struct Symbol{
	std::string id = "";
	enum TYPE type = NONE;

	Symbol(std::string id_p):id(id_p) {

	}
}Symbol;


#endif /* SYMBOL_H_ */
