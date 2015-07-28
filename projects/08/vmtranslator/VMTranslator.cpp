#include "VMTranslator.h"

VmTranslator::VmTranslator(const std::string& file, const std::string& output):
input_files_(),
output_filename_(output),
writer_(output)
{
	input_files_.push_back(file); 	// Adding the only file to the vector.
}

VmTranslator::VmTranslator(const std::vector<std::string>& files, const std::string& output):
input_files_(files),
output_filename_(output),
writer_(output)
{
	writer_.writeInit();
}


void VmTranslator::translateFile(const std::string& f) {
	Parser p(f);
	// If f is a path, get only the file name.
	std::string name(f);
	if (f.find("/"))
		writer_.setFileName(f.substr(f.find_last_of("/")+1, std::string::npos));
	else
		writer_.setFileName(f);

	while (p.hasMoreCommands()) {
		p.advance();
		CommandType ct = p.commandType();
		if (ct == C_PUSH || ct == C_POP) {
			writer_.writePushPop(ct, p.arg1(), p.arg2());
		}
		else if (ct == C_ARITHMETIC) {
			writer_.writeArithmetic(p.arg1());
		}
		else if (ct == C_LABEL) {
			writer_.writeLabel(p.arg1());
		}
		else if (ct == C_GOTO) {
			writer_.writeGoto(p.arg1());
		}
		else if (ct == C_IF) {
			writer_.writeIf(p.arg1());
		}
		else if (ct == C_FUNCTION) {
			writer_.writeFunction(p.arg1(), p.arg2());
		}
		else if (ct == C_CALL) {
			writer_.writeCall(p.arg1(), p.arg2());
		}
		else if (ct == C_RETURN) {
			writer_.writeReturn();
		}
	}
}

void VmTranslator::translate() {
	std::vector<std::string>::const_iterator it;
	for (it = input_files_.begin(); it != input_files_.end(); it++) {
		std::cout << "\t Translating " << *it << std::endl;
		translateFile(*it);
	}
}