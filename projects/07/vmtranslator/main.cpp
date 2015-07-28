#include "vmtranslator.h"

int main (int argc, char* argv[]){
	/* Arguments: 1:input (2:output) */
	if (argc <= 1) {
		std::cout << "Error: Filename must be specified." << std::endl;
		std::cout << "Usage: "  << argv[0] << " input [output]" << std::endl;
		return -1;
	}
	if (argc > 3) {
		std::cout << "Error: Extra arguments." << std::endl;
		std::cout << "Usage: "  << argv[0] << " input [output]" << std::endl;
		return -1;
	}
	std::string output_filename;
	std::ofstream output_file;
	if (argc == 3) {
		std::cout << "Saving to " << argv[2] << std::endl;
		output_filename = argv[2];
		output_file.open(argv[2], std::ifstream::out);
		if (!output_file.is_open()) {
			std::cout << "Error: Creating output file." << std::endl;
			return -1;
		}
	}

	// Checking input filename and file:
	std::string input_filename(argv[1]);
	bool validExtension = (input_filename.substr(input_filename.find_last_of(".") + 1) == "vm");
	if (!validExtension) {
		std::cerr << "Error: Invalid input file." << std::endl;
		return -1;
	}
	if (!std::ifstream(input_filename).good()){
		std::cerr << "Error: Could not open input file." << std::endl;
		return -1;
	}

	// If necessary, generating output name by changing extension of input name.
	if (output_filename.empty()) {
		output_filename = input_filename.substr(0, input_filename.length()-2);
		output_filename.append("asm");
	}
	VmTranslator vt(input_filename, output_filename);
	vt.translate();

	/*Parser p(input_filename);
	CodeWriter w(output_filename);

	while (p.hasMoreCommands()) {
		p.advance();
		CommandType ct = p.commandType();
		if (ct == C_PUSH || ct == C_POP) {
			w.writePushPop(ct, p.arg1(), p.arg2());
		}
		else if (ct == C_ARITHMETIC) {
			w.writeArithmetic(p.arg1());
		}
	}*/
}