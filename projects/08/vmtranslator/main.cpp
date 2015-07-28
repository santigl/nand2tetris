#include "VMTranslator.h"
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <vector>

std::string outputPathFromInputFile(const std::string& in) {
// Replaces .vm with .asm	
// PRE: !in.empty()
	std::string out(in);
	// Striping extension:
	size_t dot = out.find_last_of(".");
	if (dot != std::string::npos) {	
		out = out.substr(0, dot+1);
	}
	// Adding new extension:
	out.append("asm");
	return out;
}

std::string outputPathFromInputDir(const std::string& in) {
// Generating the output path as input_path/last_dir_in_input_path.asm
// PRE: !in.empty()	
	std::string res(in);
	// in has a trailing '/', removing it:
	std::string temp(in.substr(0, in.length()-1));
	// Extracting the last directory name:
	size_t slash = temp.find_last_of("/");
	temp = temp.substr(slash+1, std::string::npos);
	// Appending the directory name to the path (as a filename):
	res.append(temp);
	// And adding its extension:
	res.append(".asm");
	return res;
}

bool getVMFilesInDir(const std::string& path, std::vector<std::string>& v) {
// Populates v with all .vm files in the directory pointed at by path.
// If it couldn't find any .vm files, it returns false.	
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(path.c_str())) != NULL) {
	// Scanning directory contents looking for .vm files...
  		while ((ent = readdir(dir)) != NULL) {
  			std::string entry_name = ent->d_name;
  			if (entry_name.find(".vm") != std::string::npos) {
  				std::string file_path = path;
  				file_path.append(entry_name); // path/file.vm
  				v.push_back(file_path);  // ... and adding them to vm_files.
   			}
  		}		
   		closedir(dir); 
   		return !v.empty();
  	}
	else {
  		std::cerr << "Error opening directory." << std::endl;
  		std::exit(-1);
	}
}

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

	std::string input_path(argv[1]);
	std::string output_path;
	std::ofstream output_file;
	if (argc == 3) {
		std::cout << "Saving to " << argv[2] << std::endl;
		output_path = argv[2];
		output_file.open(argv[2], std::ifstream::out);
		if (!output_file.is_open()) {
			std::cout << "Error: Creating output file." << std::endl;
			return -1;
		}
	}

	

	// Determining if input is a file or a directory:
	struct stat st_buf;
	int status = stat(input_path.c_str(), &st_buf);
	if (status != 0) {
		std::cerr << "Error: invalid input file/directory." << std::endl;
		return -1;
	}

	// REGULAR .VM FILE
	if (S_ISREG (st_buf.st_mode)) { 
		bool validExtension = (input_path.substr(input_path.find_last_of(".") + 1) == "vm");
		if (!validExtension) {
			std::cerr << "Error: Input is not a .vm file." << std::endl;
			return -1;
		}
		if (!std::ifstream(input_path).good()){
			std::cerr << "Error: Could not open input file." << std::endl;
			return -1;
		}

		std::cout << "Translating single file." << std::endl;
		// If necessary, generating output name by changing extension of input name.
		if (output_path.empty()) {
			output_path = outputPathFromInputFile(input_path);
		}

		VmTranslator vt(input_path, output_path);
		vt.translate();
		std::cout << "Writing to " << output_path << std::endl;
    }

    // DIRECTORY:
    else if (S_ISDIR (st_buf.st_mode)) {
    	std::cout << "Translating directory." << std::endl;

		// Normalizing path to have a trailing slash.
		if (input_path.back() != '/')	
			input_path.append("/");

		std::vector<std::string> vm_files;
		if (!getVMFilesInDir(input_path, vm_files)) {
			std::cerr << "Empty directory; no files to translate." << std::endl;
			return -1;
		}

		if (output_path.empty()) {
			output_path = outputPathFromInputDir(input_path);
		}
		else {
			output_path.append(output_path);
		}
		
		VmTranslator vt(vm_files, output_path);
		vt.translate();
		std::cout << "Writing to " << output_path << std::endl;
    }
    return 0;
}