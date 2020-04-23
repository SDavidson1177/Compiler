/*
 * startup.cc
 *
 *  Created on: Apr. 22, 2020
 *      Author: solomon
 */

#include "startup.h"

void initHelpers(std::ostream& file){
	// procedure for converting ascii to integers
	// ascii value must be stored at [r10]
	file << "ascii_to_int:\n";
	file << "\tsub rsp, 8\n";
	file << "\tmov [rsp], rcx\n";
	file << "\tsub rsp, 8\n";
	file << "\tmov [rsp], rdx\n";
	file << "\tmov eax, 0\n";
	file << "\tmov edx, 0\n";
	file << "\tmov cl, 10\n";
	file << "ascii_to_int_loop:\n";
	file << "\tmov dl, byte [r10]\n";
	file << "\tcmp edx, 0\n";
	file << "\tje ascii_to_int_end\n";
	file << "\tsub dl, '0'\n";
	file << "\tmul cl\n";
	file << "\tadd rax, rdx\n";
	file << "\tadd r10, 1\n";
	file << "\tcmp eax, eax\n";
	file << "\tje ascii_to_int_loop\n";
	file << "\tascii_to_int_end:\n";
	file << "\tmov rdx, [rsp]\n";
	file << "\tadd rsp, 8\n";
	file << "\tmov rcx, [rsp]\n";
	file << "\tadd rsp, 8\n";
	file << "\tret\n\n";
}


