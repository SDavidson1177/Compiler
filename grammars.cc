/*
 * grammars.cc
 *
 *  Created on: Mar. 17, 2020
 *      Author: solomon
 */

#include <iostream>
#include <stdexcept>
#include "grammars.h"
#include "errorcodes.h"
#include "symbol.h"

/* Important global variables */

std::vector<std::map<std::string, Symbol>*> cur_sym_table;
std::map<std::string, std::vector<Procedure*>> proc_map;
int VAR_STACK_OFFSET = 0;
int* STACK_MAX = nullptr;

/* Get the next token for the grammar */
std::string next(std::vector<std::pair<std::string, std::string>>& global_tokens,
		std::vector<std::pair<std::string, std::string>>::iterator& it,
		std::string expected){
	if(it == global_tokens.end() || it->second != expected){
		throw thrown_e(PARSE_ERROR, " Expected token " + expected + ", but instead saw " + it->second + ".");
	}
	std::string retval = it->first;
	++it;
	return retval;
}

std::vector<std::pair<std::string, std::string>>::iterator
update_lookahead(std::vector<std::pair<std::string, std::string>>::iterator& it,
		std::vector<std::pair<std::string, std::string>>::iterator end){
	auto lookahead = it + 1;
	if(lookahead == end){
		throw thrown_e(PARSE_ERROR, " Expected a lookahead token");
	}
	return lookahead;
}

/* Static Functions */

bool Params::first(std::string check){
	return ((check == "RPAREN") || Paramlist::first(check));
}

bool Paramlist::first(std::string check){
	return Dcl::first(check);
}

bool Dcl::first(std::string check){
	return Type::first(check);
}

bool Type::first(std::string check){
	if (check == "INT"){
		return true;
	}
	return false;
}

bool Dcls::first(std::string check){
	return Dcl::first(check);
}

bool Lvalue::first(std::string check){
	if (check == "STAR" || check == "ID" || check == "LPAREN"){
		return true;
	}
	return false;
}

bool Statement::first(std::string check){
	if (check == "IF" || check == "WHILE" || check == "DELETE" ||
			check == "PRINTLN"){
		return true;
	}
	return Lvalue::first(check);
}

/* Grammar Parent Class */
GRAMMAR_ID Grammar::getId(){
	return this->id;
}

void Grammar::print(std::ostream& out, std::string prefix){
	switch(this->id){
	case PROCEDURES:
		out << "Procedure: " << this->value << std::endl;
		break;
	default:
		out << "Unrecognized Grammar\n";
	}
}

Grammar::~Grammar(){

}

/* TYPED Parent Class */

Typed::Typed(){
	this->type = NONE;
}

enum TYPE Typed::getType(){
	return this->type;
}

/* PROCEDURES GRAMMAR */

Procedures::Procedures(){
	this->id = PROCEDURES;
	this->value = "";
	this->version = -1;
}

Procedures::~Procedures(){
	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		delete *it;
	}
}


err_code Procedures::parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
		std::vector<std::pair<std::string, std::string>>::iterator& it, int version){
	try{
		if (it->second == "INT"){
			auto lookahead = update_lookahead(it, global_tokens.end());
			if(lookahead->second == "MAIN"){
				version = 1;
				Main* main_function = new Main();
				this->grammars.emplace_back(main_function);
				main_function->parseTokens(global_tokens, it);

			}else if(lookahead->second == "ID"){
				version = 0;
				Procedure* procedure = new Procedure();
				this->grammars.emplace_back(procedure);
				procedure->parseTokens(global_tokens, it);

				Procedures* procedures = new Procedures();

				this->grammars.emplace_back(procedures);
				err_code status =  procedures->parseTokens(global_tokens, it);

				if (status != OK){
					throw thrown_e(PARSE_ERROR, " Expected main function");
				}

			}else{
				throw thrown_e(PARSE_ERROR, " Invalid syntax for a procedure");
			}
		}else{
			throw thrown_e(PARSE_ERROR, " Invalid syntax for a procedure");
		}
	}catch(thrown_e& ex){
		std::cerr << "ERROR:" << ex.message << std::endl;
		return ex.e;
	}
	return OK;
}

void Procedures::print(std::ostream& out, std::string prefix){
	out << prefix << "-----Procedures------\n";
	out << prefix << "version: " << this->version << std::endl << std::endl;

	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		(*it)->print(out, prefix + "\t");
	}
}

/* PROCEDURE GRAMMAR */

Procedure::Procedure(){
	this->sym_table = new std::map<std::string, Symbol>();
	this->id = PROCEDURE;
	this->value = "";
	this->version = -1;
}

Procedure::~Procedure(){
	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		delete *it;
	}

	delete this->sym_table;
}

bool Procedure::argumentMatch(Arglist* arglist){
	Params* params = dynamic_cast<Params*>(this->grammars.at(0));
	return params->argumentMatch(arglist);
}

bool Procedure::noParams(){
	return dynamic_cast<Params*>(this->grammars.at(0))->noParams();
}

bool Params::noParams(){
	return this->grammars.empty();
}

err_code Procedure::parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
		std::vector<std::pair<std::string, std::string>>::iterator& it, int version){
	cur_sym_table.emplace_back(this->sym_table);
	VAR_STACK_OFFSET = 0;

	next(global_tokens, it, "INT");
	this->name = next(global_tokens, it, "ID");
	next(global_tokens, it, "LPAREN");

	Params* params = new Params();
	this->grammars.emplace_back(params);
	params->parseTokens(global_tokens, it);

	next(global_tokens, it, "RPAREN");
	next(global_tokens, it, "LBRACE");
	Body* body = new Body();
	this->grammars.emplace_back(body);
	body->parseTokens(global_tokens, it);

	next(global_tokens, it, "RETURN");

	Expr* expr = new Expr();
	this->grammars.emplace_back(expr);
	expr->parseTokens(global_tokens, it);

	next(global_tokens, it, "SEMI");
	next(global_tokens, it, "RBRACE");

	cur_sym_table.pop_back();

	// Add this procedure to the global map of procedures
	auto map_location = proc_map.find(this->name);
	if (map_location == proc_map.end()){
		std::vector<Procedure*> proc_vector;
		proc_vector.emplace_back(this);

		proc_map.emplace(std::make_pair(this->name, proc_vector));
	}else{
		map_location->second.emplace_back(this);
	}

	return OK;
}

void Procedure::print(std::ostream& out, std::string prefix){
	out << prefix << "-----Procedure------\n";
	out << prefix << "version: " << this->version << std::endl;
	out << prefix << "identifier: " << this->name << std::endl << std::endl;

	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		(*it)->print(out, prefix + "\t");
	}
}

/* MAIN GRAMMAR */

Main::Main(){
	this->sym_table = new std::map<std::string, Symbol>();
	this->stack_max = 0;
	this->id = MAIN;
	this->value = "";
	this->version = 0;
}

Main::~Main(){
	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		delete *it;
	}

	delete this->sym_table;
}

err_code Main::parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
		std::vector<std::pair<std::string, std::string>>::iterator& it, int version){
	VAR_STACK_OFFSET = 0;
	STACK_MAX = &(this->stack_max);
	cur_sym_table.emplace_back(this->sym_table);

	next(global_tokens, it, "INT");
	next(global_tokens, it, "MAIN");
	next(global_tokens, it, "LPAREN");

	Dcl* dcl_1 = new Dcl();
	this->grammars.emplace_back(dcl_1);
	dcl_1->parseTokens(global_tokens, it);

	next(global_tokens, it, "COMMA");

	Dcl* dcl_2 = new Dcl();
	this->grammars.emplace_back(dcl_2);
	dcl_2->parseTokens(global_tokens, it);

	// Set the version of our main function
	// Default version 0 for both arguments being of time INT

	if (dcl_1->getType() == INT && dcl_2->getType() == INT){
		// default
	}else if(dcl_1->getType() == INT && dcl_2->getType() == INT_STAR){
		this->version = 1;
	}else{
		throw thrown_e(CONTEXT_ERROR, " Invalid types for arguments to main function. Expect (int, int) or (int, int*)");
	}

	next(global_tokens, it, "RPAREN");
	next(global_tokens, it, "LBRACE");

	Body* body = new Body();
	this->grammars.emplace_back(body);
	body->parseTokens(global_tokens, it);

	next(global_tokens, it, "RETURN");

	Expr* expr = new Expr();
	this->grammars.emplace_back(expr);
	expr->parseTokens(global_tokens, it);

	next(global_tokens, it, "SEMI");
	next(global_tokens, it, "RBRACE");

	cur_sym_table.pop_back();
	STACK_MAX = nullptr;

	return OK;
}

void Main::print(std::ostream& out, std::string prefix){
	out << prefix << "-----Main------\n";
	out << prefix << "version: " << this->version << std::endl;
	out << prefix << "identifier: main" << std::endl << std::endl;

	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		(*it)->print(out, prefix + "\t");
	}
}

/* PARAMS GRAMMAR */

Params::Params(){
	this->id = PARAMS;
	this->value = "";
	this->version = 0;
}

Params::~Params(){
	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		delete *it;
	}
}

bool Params::argumentMatch(Arglist* arglist){
	if(this->grammars.size() > 0){
		Paramlist* paramlist = dynamic_cast<Paramlist*>(this->grammars.at(0));
		return paramlist->argumentMatch(arglist);
	}
	return false;
}

err_code Params::parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
		std::vector<std::pair<std::string, std::string>>::iterator& it, int version){
	if (Paramlist::first(it->second)){
		// Parse tokens for a paramlist

		Paramlist* pl = new Paramlist();
		this->grammars.emplace_back(pl);
		pl->parseTokens(global_tokens, it);
	}

	return OK;
}

void Params::print(std::ostream& out, std::string prefix){
	out << prefix << "-----Params------\n";

	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		(*it)->print(out, prefix + "\t");
	}
}

/* PARAMLIST GRAMMAR */

Paramlist::Paramlist(){
	this->id = PARAMLIST;
	this->value = "";
	this->version = 0;
}

Paramlist::~Paramlist(){
	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		delete *it;
	}
}

bool Paramlist::argumentMatch(Arglist* arglist){
	try{
		if (this->grammars.size() != arglist->size()){
			return false;
		}

		int i = 0;
		for (auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
			if (dynamic_cast<Dcl*>(*it)->getType() != dynamic_cast<Expr*>(arglist->at(i))->getType()){
				return false;
			}
			i++;
		}
	}catch(const std::out_of_range& err){
		return false;
	}
	return true;
}

err_code Paramlist::parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
		std::vector<std::pair<std::string, std::string>>::iterator& it, int version){
	Dcl* dcl = new Dcl();
	this->grammars.emplace_back(dcl);
	dcl->parseTokens(global_tokens, it);

	if(it->second == "COMMA"){
		next(global_tokens, it, "COMMA");

		Paramlist* paramlist = new Paramlist();
		this->grammars.emplace_back(paramlist);
		paramlist->parseTokens(global_tokens, it);
	}

	return OK;
}

void Paramlist::print(std::ostream& out, std::string prefix){
	out << prefix << "-----Paramlist------\n";

	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		(*it)->print(out, prefix + "\t");
	}
}

/* DCL GRAMMAR */

Dcl::Dcl() : Typed{}{
	this->id = DCL;
	this->value = "";
	this->version = 0;
}

Dcl::~Dcl(){
	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		delete *it;
	}
}

err_code Dcl::parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
		std::vector<std::pair<std::string, std::string>>::iterator& it, int version){
	Type* type = new Type();
	this->grammars.emplace_back(type);
	type->parseTokens(global_tokens, it);

	this->type = type->getType();
	this->value = next(global_tokens, it, "ID");

	// Add this variable to the symbol table

	auto inner_scope = cur_sym_table.rbegin();


	if((*inner_scope)->find(this->value) == (*inner_scope)->end()){
		auto sym = Symbol(this->value);
		sym.type = type->getType();
		sym.offset = VAR_STACK_OFFSET;
		VAR_STACK_OFFSET -= SIZE_OF_INT;

		if(this->type == INT){
			*STACK_MAX += SIZE_OF_INT;
		}

		(*inner_scope)->emplace(std::make_pair(this->value, sym));
	}else{
		throw thrown_e(CONTEXT_ERROR, " Multiple declaration of variable \'" + this->value + "\'.");
	}

	return OK;
}

void Dcl::print(std::ostream& out, std::string prefix){
	out << prefix << "-----Declaration------\n";
	out << prefix << "Identifier: " << this->value << std::endl;

	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		(*it)->print(out, prefix + "\t");
	}
}

/* TYPE GRAMMAR */

Type::Type(){
	this->type = NONE;
	this->id = TYPE;
	this->value = "";
	this->version = 0;
}

Type::~Type(){
	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		delete *it;
	}
}

err_code Type::parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
		std::vector<std::pair<std::string, std::string>>::iterator& it, int version){
	auto lookahead = update_lookahead(it, global_tokens.end());

	this->value = next(global_tokens, it, "INT");
	this->type = INT;

	if(lookahead->second == "STAR"){
		version = 1;
		this->value += " " + next(global_tokens, it, "STAR");
		this->type = INT_STAR;
	}

	return OK;
}

void Type::print(std::ostream& out, std::string prefix){
	out << prefix << "-----Type------\n";
	out << prefix << "version: " << this->value << std::endl;

	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		(*it)->print(out, prefix + "\t");
	}
}

/* DCLS GRAMMAR */

Dcls::Dcls() : Typed{}{
	this->id = DCLS;
	this->value = "";
	this->version = 0;
}

Dcls::~Dcls(){
	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		delete *it;
	}
}

err_code Dcls::parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
		std::vector<std::pair<std::string, std::string>>::iterator& it, int version){

	Dcl* dcl = new Dcl();
	this->grammars.emplace_back(dcl);
	dcl->parseTokens(global_tokens, it);

	this->type = dcl->getType();

	next(global_tokens, it, "ASSIGN");


	if(it->second == "INT"){
		this->value = next(global_tokens, it, "INT");

		if (this->getType() != INT){
			throw thrown_e(CONTEXT_ERROR, " Type missmatch");
		}
	}else{
		next(global_tokens, it, "NULL");
		this->value = "0";

		if (this->getType() != INT_STAR){
			throw thrown_e(CONTEXT_ERROR, " Type missmatch");
		}
	}

	next(global_tokens, it, "SEMI");

	return OK;
}

void Dcls::print(std::ostream& out, std::string prefix){
	out << prefix << "-----Dcls------\n";
	out << prefix << "version: " << this->version << std::endl;

	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		(*it)->print(out, prefix + "\t");
	}
}

/* LVALUE GRAMMAR */

Lvalue::Lvalue(){
	this->type = NONE;
	this->id = LVALUE;
	this->value = "";
	this->version = 0;
}

Lvalue::~Lvalue(){
	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		delete *it;
	}
}

err_code Lvalue::parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
		std::vector<std::pair<std::string, std::string>>::iterator& it, int version){

	if(it->second == "ID"){
		std::string var = this->value = next(global_tokens, it, "ID");

		auto check_sym_table = cur_sym_table.rbegin();
		while (check_sym_table != cur_sym_table.rend()){
			auto sym = (*check_sym_table)->find(var);
			if(sym != (*check_sym_table)->end()){
				this->type = sym->second.type;
				break;
			}
				++check_sym_table;
		}

		if(check_sym_table == cur_sym_table.rend()){
			if (proc_map.find(var) == proc_map.end()){
				throw thrown_e(CONTEXT_ERROR, " Variable " + var + " not yet declared.");
			}
		}

	}else if(it->second == "STAR"){
		this->version = 1;
		next(global_tokens, it, "STAR");

		Factor* factor = new Factor();
		this->grammars.emplace_back(factor);
		factor->parseTokens(global_tokens, it);

		this->type = INT;
	}else if(it->second == "LPAREN"){
		this->version = 2;
		next(global_tokens, it, "LPAREN");

		Lvalue* lvalue = new Lvalue();
		this->grammars.emplace_back(lvalue);
		lvalue->parseTokens(global_tokens, it);

		next(global_tokens, it, "RPAREN");

		this->type = lvalue->getType();
	}else{
		throw thrown_e(PARSE_ERROR, " Expected an ID, Star or left parenthesis. Lvalue error.");
	}

	return OK;
}

void Lvalue::print(std::ostream& out, std::string prefix){
	out << prefix << "-----Lvalue------\n";
	out << prefix << "version: " << this->version << std::endl;

	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		(*it)->print(out, prefix + "\t");
	}
}

/* FACTOR GRAMMAR */

Factor::Factor(){
	this->type = NONE;
	this->id = FACTOR;
	this->value = "";
	this->version = 0;
}

Factor::~Factor(){
	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		delete *it;
	}
}

err_code Factor::parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
		std::vector<std::pair<std::string, std::string>>::iterator& it, int version){

	auto lookahead = update_lookahead(it, global_tokens.end());

	if(it->second == "ID" && lookahead->second != "LPAREN"){
		std::string var = this->value = next(global_tokens, it, "ID");

		auto check_sym_table = cur_sym_table.rbegin();
		while (check_sym_table != cur_sym_table.rend()){
				auto sym = (*check_sym_table)->find(var);
				if(sym != (*check_sym_table)->end()){
					this->type = sym->second.type;
					break;
				}
				++check_sym_table;
		}

		if(check_sym_table == cur_sym_table.rend()){
			if (proc_map.find(var) == proc_map.end()){
				throw thrown_e(CONTEXT_ERROR, " Variable " + var + " not yet declared.");
			}
		}

	}else if(it->second == "INT"){
		this->version = 1;
		this->value = next(global_tokens, it, "INT");
		this->type = INT;
	}else if(it->second == "NULL"){
		this->version = 2;
		this->value = next(global_tokens, it, "NULL");
		this->type = INT_STAR;
	}else if(it->second == "LPAREN"){
		this->version = 3;
		next(global_tokens, it, "LPAREN");

		Expr* expr = new Expr();
		this->grammars.emplace_back(expr);
		expr->parseTokens(global_tokens, it);

		next(global_tokens, it, "RPAREN");

		this->type = expr->getType();
	}else if(it->second == "AMP"){
		this->version = 4;
		next(global_tokens, it, "AMP");

		Lvalue* lvalue = new Lvalue();
		this->grammars.emplace_back(lvalue);
		lvalue->parseTokens(global_tokens, it);

		this->type = INT_STAR;
	}else if(it->second == "STAR"){
		this->version = 5;
		next(global_tokens, it, "STAR");

		Factor* factor = new Factor();
		this->grammars.emplace_back(factor);
		factor->parseTokens(global_tokens, it);

		this->type = INT;
	}else if(it->second == "NEW"){
		this->version = 6;
		next(global_tokens, it, "INT");
		next(global_tokens, it, "LBRACK");

		Expr* expr = new Expr();
		this->grammars.emplace_back(expr);
		expr->parseTokens(global_tokens, it);

		next(global_tokens, it, "RBRACK");

		this->type = INT_STAR;
	}else if(it->second == "ID"){
		this->version = 7;
		std::string proc_name = next(global_tokens, it, "ID");

		auto procedure = proc_map.find(proc_name);

		if (procedure == proc_map.end()){
			throw thrown_e(CONTEXT_ERROR, " Procedure " + proc_name + " not defined. ");
		}

		next(global_tokens, it, "LPAREN");

		if (it->second != "RPAREN"){
			Arglist* arglist = new Arglist();
			this->grammars.emplace_back(arglist);
			arglist->parseTokens(global_tokens, it);

			auto it = procedure->second.begin();

			for (; it != procedure->second.end(); ++it){
				if(dynamic_cast<Procedure*>(*it)->argumentMatch(arglist)){
					break; // our arguments match. Every thing is OK
				}
			}

			if(it == procedure->second.end()){
				throw thrown_e(CONTEXT_ERROR, " Invalid arguments used in call to " + proc_name);
			}

		}else{
			auto it = procedure->second.begin();

			for (; it != procedure->second.end(); ++it){
				if(dynamic_cast<Procedure*>(*it)->noParams()){
					break; // our arguments match. Every thing is OK
				}
			}

			if(it == procedure->second.end()){
				throw thrown_e(CONTEXT_ERROR, " Invalid arguments used in call to " + proc_name);
			}
		}

		next(global_tokens, it, "RPAREN");

		this->type = INT;
	}else{
		throw thrown_e(PARSE_ERROR, " Unexpected token " + it->second + " . Factor error.");
	}

	return OK;
}

void Factor::print(std::ostream& out, std::string prefix){
	out << prefix << "-----Factor------\n";
	out << prefix << "version: " << this->version << std::endl;
	if(this->value != ""){
		out << prefix << "value: " << this->value << std::endl;
	}

	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		(*it)->print(out, prefix + "\t");
	}
}

/* EXPR GRAMMAR */

Expr::Expr(){
	this->type = NONE;
	this->id = EXPR;
	this->value = "";
	this->version = 0;
}

Expr::~Expr(){
	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		delete *it;
	}
}

Expr& Expr::operator=(const Expr& other){
	this->version = other.version;
	this->value = other.value;

	this->grammars.clear();
	for (auto it = other.grammars.begin(); it != other.grammars.end(); ++it){
		this->grammars.emplace_back(*it);
	}

	return *this;
}

err_code Expr::parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
		std::vector<std::pair<std::string, std::string>>::iterator& it, int version){

	if(version == 0){
		// Single Factor

		Term* term = new Term();
		this->grammars.emplace_back(term);
		term->parseTokens(global_tokens, it);

		this->type = term->getType();

	}else if(version == 1){
		if (it->second == "PLUS"){
			this->op = next(global_tokens, it, "PLUS");
		}else{
			this->op = next(global_tokens, it, "MINUS");
		}

		Term* term = new Term();
		this->grammars.emplace_back(term);
		term->parseTokens(global_tokens, it);

		//Typing for addition
		if("+" == this->op){
			if (this->type == INT_STAR && term->getType() == INT_STAR){
				throw thrown_e(CONTEXT_ERROR, " Type missmatch in term. Cannot add two integer pointers together.");
			}else if (this->type == INT_STAR && term->getType() == INT){
				this->type = INT_STAR;
			}else if (this->type == INT && term->getType() == INT_STAR){
				this->type = INT_STAR;
			}else if (this->type == INT && term->getType() == INT){
				this->type = INT;
			}else if(this->type != NONE){
				this->type = term->getType();
			}
		}else if("-" == this->op){
		//Typing for subtraction
			if (this->type == INT_STAR && term->getType() == INT_STAR){
				this->type = INT;
			}else if (this->type == INT_STAR && term->getType() == INT){
				this->type = INT_STAR;
			}else if (this->type == INT && term->getType() == INT_STAR){
				throw thrown_e(CONTEXT_ERROR, " Type missmatch in term. Cannot subtract integer pointer from integer.");
			}else if (this->type == INT && term->getType() == INT){
				this->type = INT;
			}else if(this->type != NONE){
				this->type = term->getType();
			}
		}
	}

	if(it->second == "PLUS" || it->second == "MINUS"){
		Expr* expr = new Expr();
		Expr* temp = new Expr();
		*temp = *this;
		expr->grammars.emplace_back(temp);
		*this = *expr;
		expr->grammars.clear();
		delete expr;
		this->parseTokens(global_tokens, it, 1);
	}
	return OK;
}

void Expr::print(std::ostream& out, std::string prefix){
	out << prefix << "-----Expr------\n";
	out << prefix << "version: " << this->version << std::endl;
	out << prefix << "operation: " << this->op << std::endl;

	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		(*it)->print(out, prefix + "\t");
	}
}

/* TERM GRAMMAR */

Term::Term(){
	this->type = NONE;
	this->id = TERM;
	this->value = "";
	this->version = 0;
}

Term::~Term(){
	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		delete *it;
	}
}

Term& Term::operator=(const Term& other){
	this->version = other.version;
	this->value = other.value;

	this->grammars.clear();
	for (auto it = other.grammars.begin(); it != other.grammars.end(); ++it){
		this->grammars.emplace_back(*it);
	}

	return *this;
}

err_code Term::parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
		std::vector<std::pair<std::string, std::string>>::iterator& it, int version){

	if(version == 0){
		// Single Factor

		Factor* factor = new Factor();
		this->grammars.emplace_back(factor);
		factor->parseTokens(global_tokens, it);

		this->type = factor->getType();

	}else if(version == 1){
		if (it->second == "STAR"){
			this->op = next(global_tokens, it, "STAR");
		}else if(it->second == "SLASH"){
			this->op = next(global_tokens, it, "SLASH");
		}else{
			this->op = next(global_tokens, it, "PCT");
		}

		Factor* factor = new Factor();
		this->grammars.emplace_back(factor);
		factor->parseTokens(global_tokens, it);

		if(this->type != NONE && factor->getType() != this->type){
			throw thrown_e(CONTEXT_ERROR, " Type missmatch in term. Expected type " + this->type);
		}else if(this->getType() == NONE){
			this->type = factor->getType();
		}
	}

	if(it->second == "STAR" || it->second == "SLASH" || it->second == "PCT"){
		Term* term = new Term();
		Term* temp = new Term();
		*temp = *this;
		term->grammars.emplace_back(temp);
		*this = *term;
		term->grammars.clear();
		delete term;
		this->parseTokens(global_tokens, it, 1);
	}
	return OK;
}

void Term::print(std::ostream& out, std::string prefix){
	out << prefix << "-----Term------\n";
	out << prefix << "version: " << this->version << std::endl;
	out << prefix << "operation: " << this->op << std::endl;

	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		(*it)->print(out, prefix + "\t");
	}
}

/* ARGLIST GRAMMAR */

Arglist::Arglist(){
	this->id = ARGLIST;
	this->value = "";
	this->version = 0;
}

Arglist::~Arglist(){
	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		delete *it;
	}
}

Grammar* Arglist::at(int index){
	return this->grammars.at(index);
}

unsigned Arglist::size(){
	return this->grammars.size();
}

err_code Arglist::parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
		std::vector<std::pair<std::string, std::string>>::iterator& it, int version){

	Expr* expr = new Expr();
	this->grammars.emplace_back(expr);
	expr->parseTokens(global_tokens, it);

	if(it->second == "COMMA"){
		next(global_tokens, it, "COMMA");

		Arglist* arglist = new Arglist();
		this->grammars.emplace_back(arglist);
		arglist->parseTokens(global_tokens, it);
	}
	return OK;
}

void Arglist::print(std::ostream& out, std::string prefix){
	out << prefix << "-----Arglist------\n";
	out << prefix << "version: " << this->version << std::endl;

	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		(*it)->print(out, prefix + "\t");
	}
}

/* TEST GRAMMAR */

Test::Test(){
	this->id = TEST;
	this->value = "";
	this->version = 0;
}

Test::~Test(){
	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		delete *it;
	}
}

err_code Test::parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
		std::vector<std::pair<std::string, std::string>>::iterator& it, int version){

	Expr* expr_l = new Expr();
	this->grammars.emplace_back(expr_l);
	expr_l->parseTokens(global_tokens, it);

	if(it->second == "EQUALS"){
		this->op = next(global_tokens, it, "EQUALS");
	}else if(it->second == "LT"){
		this->op = next(global_tokens, it, "LT");
	}else if(it->second == "GT"){
		this->op = next(global_tokens, it, "GT");
	}else if(it->second == "NEQUALS"){
		this->op = next(global_tokens, it, "NEQUALS");
	}

	Expr* expr_r = new Expr();
	this->grammars.emplace_back(expr_r);
	expr_r->parseTokens(global_tokens, it);

	if (expr_l->getType() != expr_r->getType()){
		throw thrown_e(CONTEXT_ERROR, "Type missmatch for the test");
	}

	return OK;
}

void Test::print(std::ostream& out, std::string prefix){
	out << prefix << "-----Test------\n";
	out << prefix << "version: " << this->version << std::endl;
	out << prefix << "operation: " << this->op << std::endl;

	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		(*it)->print(out, prefix + "\t");
	}
}

/* STATEMENT GRAMMAR */

Statement::Statement(){
	this->sym_table = nullptr;
	this->id = STATEMENT;
	this->value = "";
	this->version = 0;
}

Statement::~Statement(){
	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		delete *it;
	}

	if (this->sym_table != nullptr){
		delete this->sym_table;
	}
}

err_code Statement::parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
		std::vector<std::pair<std::string, std::string>>::iterator& it, int version){

	if(it->second == "IF"){
		this->version = 1;
		this->sym_table = new std::map<std::string, Symbol>();
		cur_sym_table.emplace_back(this->sym_table);

		next(global_tokens, it, "IF");
		next(global_tokens, it, "LPAREN");

		Test* test = new Test();
		this->grammars.emplace_back(test);
		test->parseTokens(global_tokens, it);

		next(global_tokens, it, "RPAREN");
		next(global_tokens, it, "LBRACE");

		Body* body_1 = new Body();
		this->grammars.emplace_back(body_1);
		body_1->parseTokens(global_tokens, it);

		next(global_tokens, it, "RBRACE");
		if(it->second == "ELSE"){
			next(global_tokens, it, "ELSE");
			next(global_tokens, it, "LBRACE");

			Body* body_2 = new Body();
			this->grammars.emplace_back(body_2);
			body_2->parseTokens(global_tokens, it);

			next(global_tokens, it, "RBRACE");
		}

	}else if(it->second == "WHILE"){
		this->version = 2;
		this->sym_table = new std::map<std::string, Symbol>();
		cur_sym_table.emplace_back(this->sym_table);

		next(global_tokens, it, "WHILE");
		next(global_tokens, it, "LPAREN");

		Test* test = new Test();
		this->grammars.emplace_back(test);
		test->parseTokens(global_tokens, it);

		next(global_tokens, it, "RPAREN");
		next(global_tokens, it, "LBRACE");

		Body* body = new Body();
		this->grammars.emplace_back(body);
		body->parseTokens(global_tokens, it);

		next(global_tokens, it, "RBRACE");

	}else if(it->second == "PRINTLN"){
		this->version = 3;

		next(global_tokens, it, "PRINTLN");
		next(global_tokens, it, "LPAREN");

		Expr* expr = new Expr();
		this->grammars.emplace_back(expr);
		expr->parseTokens(global_tokens, it);

		next(global_tokens, it, "RPAREN");
		next(global_tokens, it, "SEMI");
	}else if(it->second == "DELETE"){
		this->version = 4;

		next(global_tokens, it, "DELETE");
		next(global_tokens, it, "LBRACK");
		next(global_tokens, it, "RBRACK");

		Expr* expr = new Expr();
		this->grammars.emplace_back(expr);
		expr->parseTokens(global_tokens, it);

		next(global_tokens, it, "SEMI");

	}else{
		/* Lvalue code */

		Lvalue* lvalue = new Lvalue();
		this->grammars.emplace_back(lvalue);
		lvalue->parseTokens(global_tokens, it);

		next(global_tokens, it, "ASSIGN");

		Expr* expr = new Expr();
		this->grammars.emplace_back(expr);
		expr->parseTokens(global_tokens, it);

		next(global_tokens, it, "SEMI");
	}

	if (this->sym_table != nullptr){
		cur_sym_table.pop_back();
	}

	return OK;
}

void Statement::print(std::ostream& out, std::string prefix){
	out << prefix << "-----Statement------\n";
	out << prefix << "version: " << this->version << std::endl;

	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		(*it)->print(out, prefix + "\t");
	}
}

/* BODY GRAMMAR */

Body::Body(){
	this->id = BODY;
	this->value = "";
	this->version = 0;
}

Body::~Body(){
	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		delete *it;
	}
}

err_code Body::parseTokens(std::vector<std::pair<std::string, std::string>>& global_tokens,
		std::vector<std::pair<std::string, std::string>>::iterator& it, int version){

	if(Dcls::first(it->second)){
		Dcls* dcl = new Dcls();
		this->grammars.emplace_back(dcl);
		dcl->parseTokens(global_tokens, it);
	}else if(Statement::first(it->second)){
		Statement* statement = new Statement();
		this->grammars.emplace_back(statement);
		statement->parseTokens(global_tokens, it);
	}else{
		return OK;
	}

	Body* body = new Body();
	this->grammars.emplace_back(body);
	body->parseTokens(global_tokens, it);

	return OK;
}

void Body::print(std::ostream& out, std::string prefix){
	out << prefix << "-----Body------\n";
	out << prefix << "version: " << this->version << std::endl;

	for(auto it = this->grammars.begin(); it != this->grammars.end(); ++it){
		(*it)->print(out, prefix + "\t");
	}
}
