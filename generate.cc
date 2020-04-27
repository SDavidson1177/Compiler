#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "grammars.h"
#include "symbol.h"

/***********************
 * Registers:
 * EAX - Used for storing the result of arithmetic operations
 * 		also used when loading the value of variables
 *
 * EBX - Store the base stack pointer
 *
 * R8/R9 - Used as additional registers for computing
 * 		arithmetic operations.
 *
 * R10 - Stores address of where to store data on the stack
 */

/* Global variables */

int CURRENT_OFFSET = 0;
bool EXPR_TERM_DOUBLE = false;

void Grammar::generate(std::vector<std::string>& data,
		std::vector<std::string>& bss,
		std::vector<std::string>& text, int type){

	// Call generate on all child grammars
	for(auto& it : this->grammars){
		it->generate(data, bss, text);
	}
}

void Main::generate(std::vector<std::string>& data,
		std::vector<std::string>& bss,
		std::vector<std::string>& text, int type){

	cur_sym_table.emplace_back(this->sym_table);

	if (this->version == 0){
		// load in integers into correct locations
		// Set base stack pointer
		text.emplace_back("mov rbx, rsp");
		text.emplace_back("sub rbx, " + std::to_string(SIZE_OF_INT));
		text.emplace_back("sub rsp, " + std::to_string(this->stack_max));

		// first int
		text.emplace_back("mov r10, rbx");
		text.emplace_back("add r10, 20");
		text.emplace_back("mov r10, [r10]");
		text.emplace_back("call ascii_to_int");
		text.emplace_back("mov [rbx], eax");

		// second int
		text.emplace_back("mov r9, rbx");
		text.emplace_back("sub r9, " + std::to_string(SIZE_OF_INT));
		text.emplace_back("mov r10, rbx");
		text.emplace_back("add r10, 28");
		text.emplace_back("mov r10, [r10]");
		text.emplace_back("call ascii_to_int");
		text.emplace_back("mov [r9], eax");

		for(auto& it : this->grammars){
			it->generate(data, bss, text);
		}

		// Exit the program
		text.emplace_back("mov eax, 1");
		text.emplace_back("int 80h");
	}else{
		std::cerr << "Main function not yet implemented for (int, int*) arguments\n";
	}

	cur_sym_table.pop_back();
}

void Dcls::generate(std::vector<std::string>& data, std::vector<std::string>& bss,
				std::vector<std::string>& text, int type){

	// Get the current offset
	dynamic_cast<Dcl*>(this->grammars.at(0))->generate(data, bss, text);
	text.emplace_back("mov r10, rbx");
	text.emplace_back("sub r10, " + std::to_string(CURRENT_OFFSET)); // go to the location in the stack
	text.emplace_back("mov eax, " + this->value);
	text.emplace_back("mov [r10], eax"); // store the value in memory

}

void Dcl::generate(std::vector<std::string>& data, std::vector<std::string>& bss,
				std::vector<std::string>& text, int type){

	// Get the current offset

	auto check_sym_table = cur_sym_table.rbegin();
	while (check_sym_table != cur_sym_table.rend()){
		auto sym = (*check_sym_table)->find(this->value);
		if(sym != (*check_sym_table)->end()){
			CURRENT_OFFSET = sym->second.offset * -1; // set the current offset of the
												 // variable on the stack
			break;
		}
		++check_sym_table;
	}
}

void Statement::generate(std::vector<std::string>& data, std::vector<std::string>& bss,
				std::vector<std::string>& text, int type){
	if (this->version == 0){
		// set the current offset
		dynamic_cast<Lvalue*>(this->grammars.at(0))->generate(data, bss, text);
		dynamic_cast<Expr*>(this->grammars.at(1))->generate(data, bss, text);

		text.emplace_back("mov r10, rbx");
		text.emplace_back("sub r10, " + std::to_string(CURRENT_OFFSET)); // go to the location in the stack
		text.emplace_back("mov [r10], eax"); // store the value in memory
	}else if(this->version == 1){ // if statement
		Test* test = dynamic_cast<Test*>(this->grammars.at(0));
		test->generate(data, bss, text);

		if (test->getVersion() == 0){ // equals
			text.emplace_back("jne if_" + this->value);
		}else if (test->getVersion() == 1){ // less than
			text.emplace_back("jge if_" + this->value);
		}else if (test->getVersion() == 2){ // greater than
			text.emplace_back("jle if_" + this->value);
		}else if (test->getVersion() == 3){ // not equals
			text.emplace_back("je if_" + this->value);
		}

		dynamic_cast<Body*>(this->grammars.at(1))->generate(data, bss, text);
		text.emplace_back("jmp if_end_" + this->value);
		text.emplace_back("if_" + this->value + ":");// end label

		if(this->grammars.size() == 3){ // there is an else clause
			dynamic_cast<Body*>(this->grammars.at(2))->generate(data, bss, text);
		}
		text.emplace_back("if_end_" + this->value + ":");// end label

	}else if(this->version == 2){ // while loop
		text.emplace_back("while_" + this->value + ":");
		Test* test = dynamic_cast<Test*>(this->grammars.at(0));
		test->generate(data, bss, text);

		if (test->getVersion() == 0){ // equals
			text.emplace_back("jne while_end_" + this->value);
		}else if (test->getVersion() == 1){ // less than
			text.emplace_back("jge while_end_" + this->value);
		}else if (test->getVersion() == 2){ // greater than
			text.emplace_back("jle while_end_" + this->value);
		}else if (test->getVersion() == 3){ // not equals
			text.emplace_back("je while_end_" + this->value);
		}

		dynamic_cast<Body*>(this->grammars.at(1))->generate(data, bss, text);
		text.emplace_back("jmp while_" + this->value);
		text.emplace_back("while_end_" + this->value + ":");
	}
}

void Expr::generate(std::vector<std::string>& data, std::vector<std::string>& bss,
					std::vector<std::string>& text, int type){
	if (this->grammars.size() == 1){
		dynamic_cast<Term*>(this->grammars.at(0))->generate(data, bss, text);
	}else{
		dynamic_cast<Term*>(this->grammars.at(1))->generate(data, bss, text);
		text.emplace_back("mov r8d, eax");
		dynamic_cast<Expr*>(this->grammars.at(0))->generate(data, bss, text);

		if(this->op == "+"){
			text.emplace_back("add eax, r8d");
		}else if(this->op == "-"){
			text.emplace_back("sub eax, r8d");
		}
		EXPR_TERM_DOUBLE = true;
	}

}

void Term::generate(std::vector<std::string>& data, std::vector<std::string>& bss,
					std::vector<std::string>& text, int type){
	if (this->grammars.size() == 1){
		dynamic_cast<Factor*>(this->grammars.at(0))->generate(data, bss, text);
	}else{
		dynamic_cast<Term*>(this->grammars.at(0))->generate(data, bss, text);
		text.emplace_back("mov r8d, eax");
		dynamic_cast<Factor*>(this->grammars.at(1))->generate(data, bss, text);
		text.emplace_back("mov r9d, eax");
		text.emplace_back("mov eax, r8d");

		if(this->op == "*"){
			text.emplace_back("mul r9d");
		}else if(this->op == "/"){
			text.emplace_back("div r9d");
		}
		EXPR_TERM_DOUBLE = true;
	}
}

void Factor::generate(std::vector<std::string>& data,
		std::vector<std::string>& bss,
		std::vector<std::string>& text, int type){

	if (this->version == 0){ // ID

		auto check_sym_table = cur_sym_table.rbegin();
		int offset = 0;

		while (check_sym_table != cur_sym_table.rend()){
				auto sym = (*check_sym_table)->find(this->value);
				if(sym != (*check_sym_table)->end()){
					offset = sym->second.offset;
					break;
				}
				++check_sym_table;
		}

		// Load variable into edx
		text.emplace_back("mov r10, rbx");
		if(offset != 0){
			int pos = offset * -1;

			text.emplace_back("sub r10, " + std::to_string(pos));
		}
		text.emplace_back("mov eax, [r10]");
	}else if(this->version == 1){
		text.emplace_back("mov eax, " + this->value);
	}else if(this->version == 2){
		text.emplace_back("mov eax, 0");
	}else if(this->version == 3){
		dynamic_cast<Expr*>(this->grammars.at(0))->generate(data, bss, text);
	}

}

void Lvalue::generate(std::vector<std::string>& data, std::vector<std::string>& bss,
			std::vector<std::string>& text, int type){
	if (this->version == 0){
		auto check_sym_table = cur_sym_table.rbegin();
		while (check_sym_table != cur_sym_table.rend()){
			auto sym = (*check_sym_table)->find(this->value);
			if(sym != (*check_sym_table)->end()){
				CURRENT_OFFSET = sym->second.offset * -1; // set the current offset of the
													 // variable on the stack
				break;
			}
			++check_sym_table;
		}
	}else if(this->version == 2){
		dynamic_cast<Lvalue*>(this->grammars.at(0))->generate(data, bss, text);
	}
}

void Test::generate(std::vector<std::string>& data, std::vector<std::string>& bss,
		std::vector<std::string>& text, int type){
	dynamic_cast<Expr*>(this->grammars.at(1))->generate(data, bss, text);
	text.emplace_back("mov edx, eax");
	dynamic_cast<Expr*>(this->grammars.at(0))->generate(data, bss, text);
	text.emplace_back("cmp eax, edx");
}

















