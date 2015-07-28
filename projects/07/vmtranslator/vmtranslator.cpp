#include "vmtranslator.h"

VmTranslator::VmTranslator(const std::string& file, const std::string& output):
input_files_(),
output_filename_(output),
writer_(output)
{
	// Adding the only file to the vector:
	input_files_.push_back(file); 
}

VmTranslator::VmTranslator(const std::vector<std::string>& files, const std::string& output):
input_files_(files),
output_filename_(output),
writer_(output)
{}


void VmTranslator::translateFile(const std::string& f) {
	Parser p(f);
	writer_.setFileName(f);
	std::string currentFunction;
	while (p.hasMoreCommands()) {
		p.advance();
		CommandType ct = p.commandType();
		if (ct == C_PUSH || ct == C_POP) {
			writer_.writePushPop(ct, p.arg1(), p.arg2());
		}
		else if (ct == C_ARITHMETIC) {
			writer_.writeArithmetic(p.arg1());
		}
	}
}

void VmTranslator::translate() {
	std::vector<std::string>::const_iterator it;
	for (it = input_files_.begin(); it != input_files_.end(); it++){
		translateFile(*it);
	}
}