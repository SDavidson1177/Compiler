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
 * R10 - Stores address of where to store data on the stack/heap
 *
 * R11 - Stores the starting address of the heap
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
		text.emplace_back("_exit:");
		text.emplace_back("mov rdi, rax");
		text.emplace_back("mov rax, 60");
		text.emplace_back("syscall");
	}else{
		std::cerr << "Main function not yet implemented for (int, int*) arguments\n";
	}

	cur_sym_table.pop_back();
}

void Dcls::generate(std::vector<std::string>& data, std::vector<std::string>& bss,
				std::vector<std::string>& text, int type){

	// Get the current offset
	dynamic_cast<Dcl*>(this->grammars.at(0))->generate(data, bss, text);
	text.emplace_back("mov eax, " + this->value);

	if (this->type == INT){
		text.emplace_back("mov [r10], eax"); // store the value in memory
	}else if(this->type == INT_STAR){
		text.emplace_back("mov [r10], rax"); // store the value in memory
	}

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

	text.emplace_back("mov r10, rbx");
	text.emplace_back("sub r10, " + std::to_string(CURRENT_OFFSET)); // go to the location in the stack
}

void Statement::generate(std::vector<std::string>& data, std::vector<std::string>& bss,
				std::vector<std::string>& text, int type){
	if (this->version == 0){
		// set the current offset
		dynamic_cast<Lvalue*>(this->grammars.at(0))->generate(data, bss, text);
		text.emplace_back("sub rsp, 8");
		text.emplace_back("mov [rsp], r10");
		Expr* expr = dynamic_cast<Expr*>(this->grammars.at(1));
		expr->generate(data, bss, text);
		text.emplace_back("mov r10, [rsp]");
		text.emplace_back("add rsp, 8");

		if (expr->getType() == INT){
			text.emplace_back("mov [r10], eax"); // store the value in memory
		}else if(expr->getType() == INT_STAR){
			text.emplace_back("mov [r10], rax"); // store the value in memory
		}
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
	}else if(this->version == 4){ // DELETE LBRACK RBRACK expr SEMI
		dynamic_cast<Expr*>(this->grammars.at(0))->generate(data, bss, text);
		text.emplace_back("mov rdi, rax");
		text.emplace_back("call free");
	}
}

void Expr::generate(std::vector<std::string>& data, std::vector<std::string>& bss,
					std::vector<std::string>& text, int type){
	if (this->grammars.size() == 1){
		dynamic_cast<Term*>(this->grammars.at(0))->generate(data, bss, text);
	}else{
		dynamic_cast<Term*>(this->grammars.at(1))->generate(data, bss, text);
		text.emplace_back("mov r8, rax");
		dynamic_cast<Expr*>(this->grammars.at(0))->generate(data, bss, text);

		if(this->op == "+"){
			text.emplace_back("add rax, r8");
		}else if(this->op == "-"){
			text.emplace_back("sub rax, r8");
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
		text.emplace_back("mov r8, rax");
		dynamic_cast<Factor*>(this->grammars.at(1))->generate(data, bss, text);
		text.emplace_back("mov r9, rax");
		text.emplace_back("mov rax, r8");

		if(this->op == "*"){
			text.emplace_back("mul r9");
		}else if(this->op == "/"){
			text.emplace_back("div r9");
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
	}else if(this->version == 1){ // NUM
		text.emplace_back("mov eax, " + this->value);
	}else if(this->version == 2){ // NULL
		text.emplace_back("mov rax, 0");
	}else if(this->version == 3){ // LPAREN expr RPAREN
		dynamic_cast<Expr*>(this->grammars.at(0))->generate(data, bss, text);
	}else if(this->version == 4){ // AMP Lvalue
		dynamic_cast<Lvalue*>(this->grammars.at(0))->generate(data, bss, text);
		text.emplace_back("mov rax, r10");
	}else if(this->version == 5){ //STAR factor
		dynamic_cast<Factor*>(this->grammars.at(0))->generate(data, bss, text);
		text.emplace_back("mov eax, [rax]");
	}else if(this->version == 6){ // NEW INT LBRACK expr RBRACK
		dynamic_cast<Expr*>(this->grammars.at(0))->generate(data, bss, text);
		text.emplace_back("mov rdi, 4");
		text.emplace_back("mul rdi");
		text.emplace_back("mov edi, eax");
		text.emplace_back("call malloc");
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
		text.emplace_back("mov r10, rbx");
		text.emplace_back("sub r10, " + std::to_string(CURRENT_OFFSET)); // go to the location in the stack
	}else if(this->version == 1){ // STAR factor
		dynamic_cast<Factor*>(this->grammars.at(0))->generate(data, bss, text);
		text.emplace_back("mov r10, [r10]");
	}else if(this->version == 2){ // LPAREN lvalue RPAREN
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

















