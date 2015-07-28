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
	void writeInit();
	void writeArithmetic(const std::string&);
	void writePushPop(CommandType, const std::string&, unsigned int);
	void writeLabel(const std::string&);
	void writeGoto(const std::string&);
	void writeIf(const std::string&);
	void writeCall(const std::string&, unsigned int);
	void writeReturn();
	void writeFunction(const std::string&, unsigned int);
	

	void close();

private:
	std::string normalizeSegmentName(const std::string&, unsigned int) const;
	void writePush(const std::string&, unsigned int);
	void writePop(const std::string&, unsigned int);
	void writePushRegister(const std::string&);
	void writePopRegister(const std::string&);

	std::string input_filename_;
	std::ofstream output_file_;
	// Counter used to generate unique labels for local jumps:
	unsigned int label_counter_;
};
#endif