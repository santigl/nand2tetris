#include "parser.h"

Parser::Parser(const std::string& filename): 
filename_(filename),
current_line_number_(0), 
commands_ ({ // Command => <CommandType, Number of Arguments>
// Stack
{"push", 	{C_PUSH,		2} },
{"pop", 	{C_POP, 		2} },
// Arithmetic
{"add", 	{C_ARITHMETIC, 	0}},
{"sub", 	{C_ARITHMETIC, 	0}},
{"neg", 	{C_ARITHMETIC, 	0}},
{"eq", 		{C_ARITHMETIC, 	0}},
{"gt", 		{C_ARITHMETIC, 	0}},
{"lt", 		{C_ARITHMETIC, 	0}},
{"and", 	{C_ARITHMETIC, 	0}},
{"or", 		{C_ARITHMETIC, 	0}},
{"not", 	{C_ARITHMETIC, 	0}},
// Program Flow
{"label", 	{C_LABEL,		1}},
{"goto", 	{C_GOTO,		1}},
{"if-goto", {C_IF,			1}},
// Function calling
{"function",{C_FUNCTION,	1}},
{"call", 	{C_CALL,		1}},
{"return", 	{C_RETURN,		0}}
})

{
	f_.open(filename.c_str(), std::ifstream::in); // (Open() wants a char* name.)
	assert(f_.is_open());
}

Parser::~Parser() {
	f_.close();
}

unsigned int Parser::currentLineNumber() const {
	return current_line_number_;
}

bool Parser::hasMoreCommands() {
	if (f_.eof()) {	// No more file to check.
		return false;
	}

	// We have to look ahead for commands, skipping comments and blank lines.
	std::istream::streampos parsed_pos 	= f_.tellg(); // Saving stream state.
	std::ios::iostate 		status 		= f_.rdstate();
	f_.clear();

	bool res 					= false;
	unsigned int lines_read 	= 0; 	// So that if there's an error it displays where.
	do {
		std::string s;
		std::getline(f_, s);
		lines_read++;
		trimLeft(s);
		if (!s.empty() && !isComment(s)) { 	// Found a command line...
			if (!validCommand(s)) { 	// ...but it's an invalid line!
				std::cerr << "Error at line ";
				std::cerr << current_line_number_ + lines_read;
				std::cerr << ". '" << s << "': Invalid line" << std::endl;
				throw std::runtime_error("Invalid line");
			}
			res = true;
			goto done;
		}
	} while (!f_.eof());

done:	
	f_.seekg(parsed_pos);		// Restoring stream to its original state.
	f_.setstate(status);
	return res;
}

void Parser::advance() {
/* PRE: hasMoreCommands() == true 
=> next non-comment line contains a valid command. */
	std::string s;
	do {
		std::getline(f_, s);
		current_line_number_++;
		trimLeft(s);
	} while (isComment(s) || s.empty());

	trimRight(s);
	// Saving the line in lower case:
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	current_line_ = s;
}

std::string Parser::currentLine() const {
	assert(!current_line_.empty());
	std::string s = current_line_;
	return s;
}

std::string Parser::arg1() const {
	assert(fields(current_line_) >= 1);
// Expected behaviour according to the spec.
	if (this->commandType() == C_ARITHMETIC)
		return field(current_line_, 0);
	else
		return field(current_line_, 1);
}

unsigned int Parser::arg2() const {
	assert(fields(current_line_) >= 2);
	return std::stoi(field(current_line_, 2));
}

void Parser::trimLeft(std::string& line) const {
// Trim left white spaces.
	const std::string blank_spaces = " \t\v\n\f\r";
	line.erase(0, line.find_first_not_of(blank_spaces));
}

void Parser::trimRight(std::string& line) const {
// Trim right white spaces and delete comments.
	const std::string blank_spaces = " \t\v\n\f\r";

	std::size_t comment_pos = line.find("//");
	if (comment_pos != 0 && comment_pos != std::string::npos) { 
			// There is a comment following a command.
			line.erase(line.find_first_of("//"), std::string::npos);
	}

	line.erase(line.find_last_not_of(blank_spaces)+1, std::string::npos);
}


bool Parser::isComment(std::string& line) const {
// PRE: The line has been trimmed of whitespaces on the left.	
	if (line.find("//") == 0)
		return true;
	else
		return false;
}

void Parser::printFileInfo(std::ostream& os) {
	std::istream::streampos parsed_pos = f_.tellg(); // Saving stream position.
	std::ios::iostate status = f_.rdstate();
	f_.clear();
	f_.seekg(0, std::ios::beg);
	unsigned int lines = 0;
	unsigned int comments = 0;
	std::string s;
	while (getline(f_, s)) {
		trimLeft(s);
		if (isComment(s))
			comments++;
		lines++;
	}
	os << "File has " << lines << " lines. "; 
	os << lines-comments << " are commands. \n";

	f_.seekg(parsed_pos);		// Restoring stream to its original position.
	f_.setstate(status);
}

bool Parser::validCommand(const std::string& s) const {
	std::string command(field(s, 0));
	std::transform(command.begin(), command.end(), command.begin(), ::tolower);

	if (commands_.count(command) > 0) {
		unsigned int args = commands_.at(command).second;
		return (args == fields(s)-1);
	}

	return false;
}

unsigned int Parser::fields(const std::string& s) const {
// PRE: f is a valid field number for s.
	int res = 0;	
	std::stringstream line(s);
	std::string temp;
	while (line >> temp) {
		res++;
	}
	return res;
}

std::string Parser::field(const std::string& s, unsigned int f) const {
// PRE: f is a valid field number for s.
	assert(f >= 0 && f < fields(s));	
	std::stringstream line(s);
	std::string temp;
	int i = f;		// It can become negative (!).
	while (i >= 0) {
		line >> temp;
		i--;
	}
	return temp;
}


CommandType Parser::commandType() const {
// PRE:	current_line_ is a validCommand()
	assert(validCommand(current_line_));
	return commands_.at(field(current_line_, 0)).first;
}