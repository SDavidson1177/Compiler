/*
 * tokens.cc
 *
 *  Created on: Mar. 14, 2020
 *      Author: solomon
 */

#include <string>
#include <vector>
#include <map>
#include "tokens.h"
#include "errorcodes.h"


using namespace std;

bool STARTED = false;
map <string, string> KEY_TOKENS;
map <string, string> TOKEN_NEEDS_SPACE;

bool char_check(char c){
	if ('a' <= c && 'z' >= c){
		return true;
	}else if('A' <= c && 'Z' >= c){
		return true;
	}else if('_' == c){
		return true;
	}
	return false;
};

bool char_check_int(char c){
	if ('0' <= c && '9' >= c){
		return true;
	}
	return false;
}

bool checkStringChar(string str, int index){
	return char_check(str[index]);
}

int startup(){
	/* Initialize the KEY_TOKENS */
	KEY_TOKENS.emplace("main", "MAIN");
	KEY_TOKENS.emplace("new", "NEW");
	KEY_TOKENS.emplace("if", "IF");
	KEY_TOKENS.emplace("elif", "ELIF");
	KEY_TOKENS.emplace("else", "ELSE");
	KEY_TOKENS.emplace("while", "WHILE");
	KEY_TOKENS.emplace("int", "INT");
	KEY_TOKENS.emplace("println", "PRINTLN");
	KEY_TOKENS.emplace("return", "RETURN");
	KEY_TOKENS.emplace("nullptr", "NULL");
	KEY_TOKENS.emplace(";", "SEMI");
	KEY_TOKENS.emplace("{", "LBRACE");
	KEY_TOKENS.emplace("}", "RBRACE");
	KEY_TOKENS.emplace("(", "LPAREN");
	KEY_TOKENS.emplace(")", "RPAREN");
	KEY_TOKENS.emplace("+", "PLUS");
	KEY_TOKENS.emplace("-", "MINUS");
	KEY_TOKENS.emplace("/", "SLASH");
	KEY_TOKENS.emplace("*", "STAR");
	KEY_TOKENS.emplace("=", "ASSIGN");
	KEY_TOKENS.emplace("==", "EQUALS");
	KEY_TOKENS.emplace("!=", "NEQUALS");
	KEY_TOKENS.emplace("<", "LT");
	KEY_TOKENS.emplace(">", "GT");
	KEY_TOKENS.emplace(",", "COMMA");
	KEY_TOKENS.emplace("&", "AMP");
	KEY_TOKENS.emplace("%", "PCT");

	/* Initialize TOKEN_NEEDS_SPACE */
//	TOKEN_NEEDS_SPACE.emplace("if", "IF");
	TOKEN_NEEDS_SPACE.emplace("elif", "ELIF");
//	TOKEN_NEEDS_SPACE.emplace("else", "ELSE");
	TOKEN_NEEDS_SPACE.emplace("while", "WHILE");
	TOKEN_NEEDS_SPACE.emplace("int", "INT");
	TOKEN_NEEDS_SPACE.emplace("println", "PRINTLN");


	return OK;
}

int tokenize(std::istream& in, std::ostream& out, std::vector <std::pair<std::string, std::string>>& token_vector){
	char c;
	string token_string;
	bool was_set = false;
	string token_type = "NONE";
	int expect_count = 0;
	bool can_be_id = true;

	while(in >> noskipws >> c){

		if (STARTED){
			if(c == ' ' || c == '\t' || c == 10){ // Tokens separated by a space or tab
				// Enter a new token if we reach a white space

				if (token_type == "EX"){
					// We have an invalid token
					cerr << "ERROR: Invalid token " << token_string << endl;
					return TOKEN_ERROR;
				}

				STARTED = false;
				was_set = false;
				can_be_id = true;
//				out << "Got token: " << token_string  << " type " << token_type << endl;
				std::pair<std::string, std::string> val = std::make_pair(token_string, token_type);
				token_vector.emplace_back(val);
				token_string = "";
				token_type = "NONE";

				/* Skip all whitespaces */
				while(in >> c && (c == 10 || c == ' ' || c == '\t')){}

				if (in.fail()){
					break;
				}

			}else if(token_type == "ID" && !char_check(c) && !char_check_int(c)){
				// We received an invalid character for a symbol

				STARTED = false;
				was_set = false;
				can_be_id = true;
//				out << "Got token: " << token_string << " type " << token_type << endl;
				std::pair<std::string, std::string> val = std::make_pair(token_string, token_type);
				token_vector.emplace_back(val);
				token_string = "";
				token_type = "NONE";

				if(c != 10 && c != ' ' && c != '\t'){
					in.putback(c);
				}
				continue;
			}else if(token_type == "INT" && !char_check_int(c)){
				// We receive an invalid character for a integer

				if (char_check(c)){
					cerr << "Token Error: Invalid ID - ID cannot start with numbers";
					return TOKEN_ERROR;
				}

				STARTED = false;
				was_set = false;
				can_be_id = true;
//				out << "Got token: " << token_string << " type " << token_type << endl;
				std::pair<std::string, std::string> val = std::make_pair(token_string, token_type);
				token_vector.emplace_back(val);
				token_string = "";
				token_type = "NONE";

				if(c != 10 && c != ' ' && c != '\t'){
					in.putback(c);
				}
				continue;
			}
		}

		if (!STARTED){
			if (char_check(c)){
				token_type = "ID";
			}else if(char_check_int(c)){
				token_type = "INT";
				can_be_id = false;
			}else{
				token_type = "EX";
				can_be_id = false;
				// Tokens that are invalid on their own but that can be combined
				// with other characters to form a valid token (Ex. '!' is invalid
				// but '!=' is not).
				if(c == '!'){
					expect_count = 2;
				}
			}
		}

		STARTED = true;
		string tester = token_string + c;

		if(can_be_id){
			if(!char_check(c) && !char_check_int(c)){
				can_be_id = false;
			}
		}

		if(!was_set && KEY_TOKENS.find(tester) != KEY_TOKENS.end()){
			// we see a valid token
			was_set = true;
			token_type = KEY_TOKENS.find(tester)->second;

		}else if(was_set && KEY_TOKENS.find(tester) != KEY_TOKENS.end()){

			token_type = KEY_TOKENS.find(tester)->second;

		}else if(was_set && KEY_TOKENS.find(tester) == KEY_TOKENS.end() &&
				TOKEN_NEEDS_SPACE.find(token_string) == TOKEN_NEEDS_SPACE.end()){// end of a token (no space)
			if(!can_be_id){
				was_set = false;
				STARTED = false;
				can_be_id = true;
//				out << "Got token: " << token_string << " type " << token_type << endl;
				std::pair<std::string, std::string> val = std::make_pair(token_string, token_type);
				token_vector.emplace_back(val);
				token_type = "NONE";
				token_string = "";
				if(c != 10 && c != ' ' && c != '\t'){
					in.putback(c);
				}
				continue;
			}else{
				was_set = false;
				token_type = "ID";
			}
		}else if(was_set && KEY_TOKENS.find(tester) == KEY_TOKENS.end() &&
				TOKEN_NEEDS_SPACE.find(token_string) != TOKEN_NEEDS_SPACE.end()){
			token_type = "ID";
			was_set = false;
		}else if(token_type == "EX" && expect_count == 0){
			cerr << "ERROR: Invalid token " << token_string << endl;
			return TOKEN_ERROR;
		}

		if (expect_count > 0){
			expect_count--;
		}
		token_string += c;
	}

	if(token_type == "EX"){
		cerr << "ERROR: Invalid final token\n";
		return TOKEN_ERROR;

	}if (token_type != "NONE"){
//		out << "Got token: " << token_string << " type " << token_type << endl;
		std::pair<std::string, std::string> val = std::make_pair(token_string, token_type);
		token_vector.emplace_back(val);
	}
	return OK;
}



