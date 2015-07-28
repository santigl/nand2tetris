#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <limits>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <stdexcept>
#include <unordered_map>
#include <tuple>
#include "command.h"

class Parser {

public:	
	Parser(const std::string&);
	~Parser();
	bool 			hasMoreCommands();
	void 			advance();
	// All functions return lower case text.
	CommandType		commandType() const;
	std::string 	arg1() const;
	unsigned int	arg2() const;

	// For error displaying:
	std::string 	currentLine() const;
	unsigned int 	currentLineNumber() const;
	void 			printFileInfo(std::ostream&);

	unsigned int 	fields(const std::string&) const;
	std::string 	field(const std::string&, unsigned int) const;
private:
	bool 			isComment(std::string&) const;
	bool 			isCommand(std::string&) const;
	bool 			validCommand(const std::string&) const;
	void 			trimLeft(std::string&) const;
	void 			trimRight(std::string&) const;
	

	std::string 	filename_;
	std::ifstream 	f_;
	std::string 	current_line_;
	unsigned int 	current_line_number_;
	// Command => <CommandType, Number of Arguments>
	std::unordered_map<	std::string, 
						std::pair<CommandType, unsigned int> > commands_;
};

#endif