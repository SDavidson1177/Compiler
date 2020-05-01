#include "asmbuffer.h"

AsmBuffer::AsmBuffer(std::ofstream& out, unsigned cap) : out(out), capacity(cap){

}

AsmBuffer::~AsmBuffer(){

}

void AsmBuffer::emplace_back(std::string&& arg){
	this->vec.emplace_back(arg);

	if(this->vec.size() == this->capacity){
		for (auto& it : this->vec){
			this->out << '\t' << it << std::endl;
		}
		this->vec.clear();
	}
}

std::vector<std::string>::iterator AsmBuffer::begin(){
	return this->vec.begin();
}

std::vector<std::string>::iterator AsmBuffer::end(){
	return this->vec.end();
}
