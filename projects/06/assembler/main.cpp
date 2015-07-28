#include "assembler.h"

int main(int argc, char* argv[]){
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

	// Opening input:
	std::string input_filename(argv[1]);
	if (!std::ifstream(input_filename).good()){
		std::cerr << "Error: Input file." << std::endl;
		return -1;
	}

	// Translating...
	try {
		Assembler a(input_filename);
		if (!output_filename.empty()) {
			a.translate(output_file);
			output_file.close();
		}
		else {
			a.translate(std::cout);
		}
	} 
	catch(std::runtime_error& e) {
		std::cerr << "Exiting... (" << e.what() << ")" << std::endl;
		return -1;	
	}

	return 0;
}