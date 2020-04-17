/*
 * start.cc
 *
 *  Created on: Mar. 14, 2020
 *      Author: solomon
 */

#include <iostream>
#include <vector>
#include "tokens.h"
#include "grammars.h"

using namespace std;

vector<pair<string, string>> global_tokens;

int main(int argc, char* argv[]){
	startup();

	if (argc < 2){
		tokenize(cin, cout, global_tokens);
		for (auto it = global_tokens.begin(); it != global_tokens.end(); ++it){
			cout << "(" << it->first << ", " << it->second << ")\n";
		}

		Procedures proc;
		auto tokens_it = global_tokens.begin();
		err_code parse_status = proc.parseTokens(global_tokens, tokens_it);

		if (parse_status != OK){
			cerr << "ERROR: Fatal parser error\n";
			return -1;
		}

		proc.print(cout, "");
	}

	return 0;
}


