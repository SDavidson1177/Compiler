/*
 * start.cc
 *
 *  Created on: Mar. 14, 2020
 *      Author: solomon
 */

#include <iostream>
#include <fstream>
#include <vector>
#include "tokens.h"
#include "grammars.h"
#include "startup.h"

using namespace std;

vector<pair<string, string>> global_tokens;
vector<string> data_segment;
vector<string> bss_segment;
vector<string> text_segment;


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

		/* Print the parse tree */
		proc.print(cout, "");

		/* Generate Assembly instructions */
		ofstream out_file;
		out_file.open("a.asm");
		out_file << "; Auto-generated x86 assembly file\n";
		out_file << "section .text\n";
		out_file << "global _start\n";

		/* Start up */
		initHelpers(out_file);

		data_segment.emplace_back("section .data");
		bss_segment.emplace_back("section .bss");
		text_segment.emplace_back("_start:");

		proc.generate(data_segment, bss_segment, text_segment);

		bool section_start = true;
		for (auto& it : text_segment){
			if (!section_start){
				out_file << '\t';
			}
			section_start = false;
			out_file << it << endl;
		}

		section_start = true;

		for (auto& it : bss_segment){
			if (!section_start){
				out_file << '\t';
			}
			section_start = false;
			out_file << it << endl;
		}

		section_start = true;

		for (auto& it : data_segment){
			if (!section_start){
				out_file << '\t';
			}
			section_start = false;
			out_file << it << endl;
		}
		out_file.close();
	}

	return 0;
}


