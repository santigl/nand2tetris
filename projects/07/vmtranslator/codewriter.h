#ifndef CODEWRITER_H
#define CODEWRITER_H
#include <bitset>
#include <iostream>
#include <fstream>
#include "command.h"
#include <cassert>
#include <algorithm>

class CodeWriter {
public:
	CodeWriter(const std::string&);
	void setFileName(const std::string&);
	void writeArithmetic(const std::string&);
	void writePushPop(CommandType, const std::string&, unsigned int);
	void close();

private:
	std::string normalizeSegmentName(const std::string&, unsigned int) const;
	void writePush(const std::string&, unsigned int);
	void writePop(const std::string&, unsigned int);

	std::string input_filename_;
	std::ofstream output_file_;
	// Counter used to generate unique labels for local jumps:
	unsigned int label_counter_;
};
#endif