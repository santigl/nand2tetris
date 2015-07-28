#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <limits>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <stdexcept>

enum Command {A_COMMAND, C_COMMAND, L_COMMAND};
// Immediate for loading into A, Comp. instruction or symbol, respectively.

class Parser {

public:	
	Parser(const std::string&);
	~Parser();
	bool 			hasMoreCommands();
	void 			advance();
	Command 		commandType() const;
	std::string 	symbol() const;
	std::string 	dest() const;
	std::string 	comp() const;
	std::string 	jump() const;

	// For error displaying:
	std::string 	currentLine() const;
	unsigned int 	currentLineNumber() const;

private:
	bool 			isComment(std::string&) const;
	bool 			isCommand(std::string&) const;
	bool 			validCommandFormat(const std::string&) const;
	void 			trimLeft(std::string&) const;
	void 			trimRight(std::string&) const;
	void 			printFileInfo(std::ostream&);

	std::string 	filename_;
	std::ifstream 	f_;
	std::string 	current_line_;
	unsigned int 	current_line_number_;
};