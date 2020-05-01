/*
 * grammars.h
 *
 *  Created on: Mar. 17, 2020
 *      Author: solomon
 */

#ifndef GRAMMARS_H_
#define GRAMMARS_H_

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "errorcodes.h"

const int SIZE_OF_INT = 4;
const int SIZE_OF_INT_STAR = 8;

struct Symbol;

class Arglist;

enum GRAMMAR_ID {
	PROCEDURES,
	PROCEDURE,
	MAIN,
	PARAMS,
	PARAMLIST,
	DCL,
	DCLS,
	TYPE,
	LVALUE,
	FACTOR,
	EXPR,
	TERM,
	ARGLIST,
	TEST,
	STATEMENT,
	BODY
};

enum TYPE {
	NONE,
	INT,
	INT_STAR
};

class Grammar{
protected:
	GRAMMAR_ID id;
	int version; // Version from order in CFG file
	std::string value;
	std::vector<Grammar*> grammars;

public:

	GRAMMAR_ID getId();

	virtual void print(std::ostream& out, std::string prefix) = 0;

	virtual ~Grammar();

	virtual err_code parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
			std::vector<std::pair<std::string, std::string>>::iterator& it, int version = 0) = 0;
	virtual void generate(std::vector<std::string>& data, std::vector<std::string>& bss,
			std::vector<std::string>& text, int type = 0);
	virtual int getVersion();
};

class Typed : public Grammar{
protected:
	enum TYPE type;
public:

	Typed();

	enum TYPE getType();
	virtual void print(std::ostream& out, std::string prefix) = 0;
	virtual err_code parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
				std::vector<std::pair<std::string, std::string>>::iterator& it, int version = 0) = 0;
};

class Procedures : public Grammar{
public:
	Procedures();
	~Procedures();

	err_code parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
				std::vector<std::pair<std::string, std::string>>::iterator& it, int version = 0) override;
	void print(std::ostream& out, std::string prefix) override;
};

class Procedure : public Grammar{
	std::map<std::string, Symbol>* sym_table;
public:
	std::string name;
	int param_max;
	int symbol_max;
	int first_dcl_size;

	Procedure();
	~Procedure();

	bool argumentMatch(Arglist* arglist);
	bool noParams();
	err_code parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
				std::vector<std::pair<std::string, std::string>>::iterator& it, int version = 0) override;
	void print(std::ostream& out, std::string prefix) override;
	void generate(std::vector<std::string>& data, std::vector<std::string>& bss,
			std::vector<std::string>& text, int type = 0) override;
};

class Main : public Grammar{
	std::map<std::string, Symbol>* sym_table;
public:
	int stack_max;

	Main();
	~Main();

	err_code parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
				std::vector<std::pair<std::string, std::string>>::iterator& it, int version = 0) override;
	void print(std::ostream& out, std::string prefix) override;
	void generate(std::vector<std::string>& data, std::vector<std::string>& bss,
			std::vector<std::string>& text, int type = 0) override;
};

class Params : public Grammar{
public:

	Params();
	~Params();

	bool argumentMatch(Arglist* arglist);
	bool noParams();
	err_code parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
				std::vector<std::pair<std::string, std::string>>::iterator& it, int version = 0) override;
	void print(std::ostream& out, std::string prefix) override;
	static bool first(std::string check);
};

class Paramlist : public Grammar{
public:

	Paramlist();
	~Paramlist();

	bool argumentMatch(Arglist* arglist);
	err_code parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
				std::vector<std::pair<std::string, std::string>>::iterator& it, int version = 0) override;
	void print(std::ostream& out, std::string prefix) override;
	static bool first(std::string check);
};

class Dcl : public Typed{
public:

	Dcl();
	~Dcl();

	err_code parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
				std::vector<std::pair<std::string, std::string>>::iterator& it, int version = 0) override;
	void print(std::ostream& out, std::string prefix) override;
	static bool first(std::string check);
	void generate(std::vector<std::string>& data, std::vector<std::string>& bss,
							std::vector<std::string>& text, int type = 0) override;
};

class Type : public Typed{
public:

	Type();
	~Type();

	err_code parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
				std::vector<std::pair<std::string, std::string>>::iterator& it, int version = 0) override;
	void print(std::ostream& out, std::string prefix) override;
	static bool first(std::string check);
};

class Dcls : public Typed{
public:

	Dcls();
	~Dcls();

	err_code parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
				std::vector<std::pair<std::string, std::string>>::iterator& it, int version = 0) override;
	void print(std::ostream& out, std::string prefix) override;
	static bool first(std::string check);
	void generate(std::vector<std::string>& data, std::vector<std::string>& bss,
							std::vector<std::string>& text, int type = 0) override;
};

class Lvalue : public Typed{
public:

	Lvalue();
	~Lvalue();

	err_code parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
				std::vector<std::pair<std::string, std::string>>::iterator& it, int version = 0) override;
	void print(std::ostream& out, std::string prefix) override;
	static bool first(std::string check);
	void generate(std::vector<std::string>& data, std::vector<std::string>& bss,
				std::vector<std::string>& text, int type = 0) override;
};

class Factor : public Typed{
public:

	Factor();
	~Factor();

	err_code parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
				std::vector<std::pair<std::string, std::string>>::iterator& it, int version = 0) override;
	void print(std::ostream& out, std::string prefix) override;
	void generate(std::vector<std::string>& data, std::vector<std::string>& bss,
			std::vector<std::string>& text, int type = 0) override;
};

class Expr : public Typed{
	std::string op;
public:

	Expr();
	~Expr();

	Expr& operator=(const Expr& other);

	err_code parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
			std::vector<std::pair<std::string, std::string>>::iterator& it, int version = 0) override;
	void print(std::ostream& out, std::string prefix) override;
	static bool first(std::string check);
	void generate(std::vector<std::string>& data, std::vector<std::string>& bss,
						std::vector<std::string>& text, int type = 0) override;
};

class Term : public Typed{
	std::string op;
public:

	Term();
	~Term();

	Term& operator=(const Term& other);

	err_code parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
			std::vector<std::pair<std::string, std::string>>::iterator& it, int version = 0) override;
	void print(std::ostream& out, std::string prefix) override;
//	static bool first(std::string check);
	void generate(std::vector<std::string>& data, std::vector<std::string>& bss,
						std::vector<std::string>& text, int type = 0) override;
};

class Arglist : public Grammar{
public:

	Arglist();
	~Arglist();

	Grammar* at(int index);
	unsigned size();
	err_code parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
			std::vector<std::pair<std::string, std::string>>::iterator& it, int version = 0) override;
	void print(std::ostream& out, std::string prefix) override;
//	static bool first(std::string check);
	void generate(std::vector<std::string>& data, std::vector<std::string>& bss,
				std::vector<std::string>& text, int type = 0) override;
};

class Test : public Grammar{
	std::string op;
public:

	Test();
	~Test();

	err_code parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
			std::vector<std::pair<std::string, std::string>>::iterator& it, int version = 0) override;
	void print(std::ostream& out, std::string prefix) override;
//	static bool first(std::string check);
	void generate(std::vector<std::string>& data, std::vector<std::string>& bss,
				std::vector<std::string>& text, int type = 0) override;
};

class Statement : public Grammar{
	std::map<std::string, Symbol>* sym_table;
public:

	Statement();
	~Statement();

	err_code parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
			std::vector<std::pair<std::string, std::string>>::iterator& it, int version = 0) override;
	void print(std::ostream& out, std::string prefix) override;
	static bool first(std::string check);
	void generate(std::vector<std::string>& data, std::vector<std::string>& bss,
					std::vector<std::string>& text, int type = 0) override;
};

class Body : public Grammar{
public:

	Body();
	~Body();

	err_code parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
			std::vector<std::pair<std::string, std::string>>::iterator& it, int version = 0) override;
	void print(std::ostream& out, std::string prefix) override;
//	static bool first(std::string check);
};

extern std::vector<std::map<std::string, Symbol>*> cur_sym_table;
extern std::map<std::string, std::vector<Procedure*>> proc_map;
#endif /* GRAMMARS_H_ */
