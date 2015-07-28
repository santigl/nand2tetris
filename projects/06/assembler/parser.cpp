#include "parser.h"

Parser::Parser(const std::string& filename): 
filename_(filename),
current_line_number_(0)
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
			if (!validCommandFormat(s)) { 	// ...but it's an invalid line!
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
	current_line_ = s;
}

std::string Parser::currentLine() const {
	assert(!current_line_.empty());
	std::string s = current_line_;
	return s;
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

bool Parser::validCommandFormat(const std::string& s) const {
	if (s.find("@") == 0) // Load into A command
		return true;
	if (s.find("(") != std::string::npos) // Label
		return true;
	if (s.find("=") != std::string::npos || s.find(";") != std::string::npos) // Comp. command
		return true;
	return false;
}

Command Parser::commandType() const {
// PRE:	current_line_ is a validCommand()
	if (current_line_.find("@") == 0)
		return A_COMMAND;
	if (current_line_.find("(") != std::string::npos)
		return L_COMMAND;
	else
		return C_COMMAND;
}

std::string Parser::symbol() const {
// PRE: current_line_ has commandType == A_COMMAND or L_COMMAND
// POST: dest in upper.	Empty string if null.
	assert(commandType() == A_COMMAND || commandType() == L_COMMAND);
	std::string s = current_line_;
	s.erase(0,1); 				// Erasing '@' or '('
	size_t p = s.find(")");
	if (p != std::string::npos) // Looking for the ')'.
		s.erase(p, std::string::npos);

	std::transform(s.begin(), s.end(), s.begin(), ::toupper);		
	return s;
}

std::string Parser::dest() const {
// PRE: current_line_ has commandType == C_COMMAND
// POST: dest in UPPERCASE. Empty string if null.
	assert(commandType() == C_COMMAND);
	std::string s = current_line_;
	size_t e = s.find("=");
	if (e == std::string::npos)
		s = ""; // Dest = null.
	else { 
		s.erase(s.find("="), std::string::npos);
	}
	std::transform(s.begin(), s.end(), s.begin(), ::toupper);
	return s;
}

std::string Parser::comp() const {
// PRE: current_line_ has commandType == C_COMMAND
// POST: comp in UPPERCASE. (Empty string if none.)
	assert(commandType() == C_COMMAND);
	std::string s = current_line_;
	size_t p = s.find(";");
	if (p != std::string::npos)
		s.erase(p+1, std::string::npos);

	p = s.find("=");
	if (p != std::string::npos)
		s.erase(0, p+1);

	size_t end = s.find_last_not_of(";"); 
	std::transform(s.begin(), s.end(), s.begin(), ::toupper);
	return s.substr(0, end+1);
}

std::string Parser::jump() const {
// PRE: current_line_ has commandType == C_COMMAND
// POST: jmp in UPPERCASE. Empty string if none.
	assert(commandType() == C_COMMAND);
	std::string s = current_line_;
	size_t p = s.find(";");
	if (p != std::string::npos) {
		s.erase(0, p+1);
		std::transform(s.begin(), s.end(), s.begin(), ::toupper);
		return s;
	}
	// No jmp mnemonic.
	return "";
}