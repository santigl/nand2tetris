#ifndef VMTRANSLATOR_H
#define VMTRANSLATOR_H

#include "Parser.h"
#include "CodeWriter.h"
#include <string>
#include <vector>

class VmTranslator {
public:
	// Translate one file.
	VmTranslator(const std::string&, const std::string&);
	// Translate many files.
	VmTranslator(const std::vector<std::string>& files, const std::string&);
	void translate();

private:
	void translateFile(const std::string&);

	std::vector<std::string> 	input_files_;
	std::string 				output_filename_;
	CodeWriter 					writer_;
};

#endif