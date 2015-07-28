#include "parser.h"
#include <iostream>
#include <unordered_map>
#include <string>
#include <bitset>
#include "code.h"

class Assembler {

public:
	Assembler(const std::string&);
	void translate(std::ostream&) const;

private:
	std::string 								input_filename_;
	uint16_t 									stack_pointer_;
	std::unordered_map<std::string, uint16_t> 	symbol_table_;
};