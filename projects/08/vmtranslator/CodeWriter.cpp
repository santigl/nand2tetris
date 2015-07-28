#include "CodeWriter.h"

CodeWriter::CodeWriter(const std::string& out): label_counter_(0) {
	output_file_.open(out, std::ofstream::out);
	assert(output_file_.is_open());
}

void CodeWriter::setFileName(const std::string& in){
	input_filename_ = in;
}

void CodeWriter::writeInit() {
	using namespace std;
	// Set SP to 256:
	output_file_ << "@256" << endl;
	output_file_ << "D=A" << endl;
	output_file_ << "@SP" << endl;
	output_file_ << "M=D" << endl;

	writeCall("sys.init", 0);
}

void CodeWriter::writeArithmetic(const std::string& s) {
// PRE: s comes straight from the parser and is all lower case.
/* -- Stack -- 
		x
		y
SP -->		
	----------	
Binary operations: x (op.) y
*/
	using namespace std;

	// Getting y's address:
	output_file_ << "@SP" << endl;
	output_file_ << "A=M-1" << endl;		// A -> y

	// Unary operations (simpler cases).
	if (s == "neg") {
		output_file_ << "M=-M" << endl;
		return;
	}
	if (s == "not") {
		output_file_ << "M=!M" << endl;
		return;
	}

	// Binary operations: D contains y, A points to x.
	output_file_ << "D=M" << endl;		// D = y
	output_file_ << "A=A-1" << endl; 	// A -> x

	if (s == "add") {					// [SP-1] <- result
		output_file_ << "M=M+D" << endl;	
	}
	else if (s == "sub") {
		output_file_ << "M=M-D" << endl;
	}
	else if (s == "and") {
		output_file_ << "M=D&M" << endl;
	}
	else if (s == "or") {
		output_file_ << "M=D|M" << endl;
	}
	else if (s == "eq") {	
		output_file_ << "D=M-D" << endl;	// D = x-y

		output_file_ << "@eq_equals" << label_counter_ << endl;
		output_file_ << "D;JEQ" << endl;
		output_file_ << "D=0"	<< endl;	// D = false
		output_file_ << "@eq_done" << label_counter_ << endl;
		output_file_ << "0;JMP" << endl;

		output_file_ << "(eq_equals" << label_counter_ << ")" << endl;
		output_file_ << "D=-1" << endl;		// D = true

		output_file_ << "(eq_done" << label_counter_ << ")" << endl;
		output_file_ << "@SP" << endl;
		output_file_ << "A=M-1" << endl;	// A = [SP]-1
		output_file_ << "A=A-1" << endl; 	// A--
		output_file_ << "M=D" << endl;		// [SP]-2 <- result
	}
	else if (s == "lt") {
		output_file_ << "D=M-D" << endl;	// D = x-y

		output_file_ << "@lt_true" << label_counter_ << endl;
		output_file_ << "D;JLT" << endl;    // x-y < 0 => x < y => true
		output_file_ << "D=0"	<< endl;	// D = false
		output_file_ << "@lt_done" << label_counter_<<  endl;
		output_file_ << "0;JMP" << endl;

		output_file_ << "(lt_true" << label_counter_ << ")" << endl;
		output_file_ << "D=-1" << endl;		// D = true

		output_file_ << "(lt_done" << label_counter_ << ")" << endl;
		output_file_ << "@SP" << endl;
		output_file_ << "A=M-1" << endl;
		output_file_ << "A=A-1" << endl; 	// A = [SP]-2
		output_file_ << "M=D" << endl;		// [SP]-2 <- result
	}	

	else if (s == "gt") {
		output_file_ << "D=M-D" << endl;	// D = x-y

		output_file_ << "@gt_true" << label_counter_ << endl;
		output_file_ << "D;JGT" << endl;    // x-y > 0 => x > y => true
		output_file_ << "D=0"	<< endl;	// D = false
		output_file_ << "@gt_done" << label_counter_<<  endl;
		output_file_ << "0;JMP" << endl;

		output_file_ << "(gt_true" << label_counter_ << ")" << endl;
		output_file_ << "D=-1" << endl;		// D = true

		output_file_ << "(gt_done" << label_counter_ << ")" << endl;
		output_file_ << "@SP" << endl;
		output_file_ << "A=M-1" << endl;
		output_file_ << "A=A-1" << endl; 	// A = [SP] - 2
		output_file_ << "M=D" << endl;		// [SP]-2 <- result
	}
	// Decrementing the stack pointer:
	output_file_ << "@SP" << endl;
	output_file_ << "M=M-1" << endl;	

	label_counter_++;
}

void CodeWriter::writePushPop(CommandType c, const std::string& s, unsigned int index) {
// PRE: s comes straight from the parser and is all lower case.
	using namespace std;
	std::string segment(normalizeSegmentName(s, index));

	if (c == C_PUSH) {
		writePush(segment, index);
	}
	else if (c == C_POP) { 
		writePop(segment, index);
	}
}

std::string CodeWriter::normalizeSegmentName(const std::string& s, unsigned int index) const {
// Translating the VM-syntax segment descriptors to assembly syntax.
// PRE: The string comes straight from the parser and is all lower case.
	if (s == "local")
		return "LCL";
	if (s == "argument")
		return "ARG";
	if (s == "this")
		return "THIS";
	if (s == "that")
		return "THAT";

	return s;
}

void CodeWriter::close() {
	output_file_.close();
}

void CodeWriter::writePush(const std::string& segment, unsigned int index) {
	using namespace std;
	if (segment == "constant") {
		output_file_ << "@" << index << endl;
		output_file_ << "D=A" << endl;			// D = constant.
	}
	// TEMP segment: indexed access to the pointer.
	else if (segment == "temp") {
		output_file_ << "@R5" << endl;	
		output_file_ << "D=A" << endl;
		output_file_ << "@" << index << endl; 	// A = offset
		output_file_ << "A=D+A" << endl;		// A = base+offset
		output_file_ << "D=M" << endl;			// D = element to push			
	}
	// POINTER segment selects between THIS and THAT (no dereferences).
	else if (segment == "pointer") {
		if (index == 0)
			output_file_ << "@THIS" << endl;
		else
			output_file_ << "@THAT" << endl;

		output_file_ << "D=M" << endl;
	}
	// Static variables.
	else if (segment == "static") {
		output_file_ << "@" << input_filename_ << "." << index << endl; // xxx.i
		output_file_ << "D=M" << endl;
	}
	else {
		output_file_ << "@" << segment << endl;	
		output_file_ << "D=M" << endl;			// D = segment base
		output_file_ << "@" << index << endl; 	// A = offset
		output_file_ << "A=D+A" << endl;		// A = base+offset
		output_file_ << "D=M" << endl;			// D = element to push
	}
	// D = constant/element to push
	output_file_ << "@SP" << endl;				// A -> top of stack
	output_file_ << "A=M" << endl;			
	output_file_ << "M=D" << endl;				// Writing the element
	output_file_ << "@SP" << endl;
	output_file_ << "M=M+1" << endl;			// SP++
}

void CodeWriter::writePop(const std::string& segment, unsigned int index) {
	using namespace std;

	output_file_ << "@SP" << endl;
	output_file_ << "A=M-1" << endl;		// A -> element
	output_file_ << "D=M" << endl;			// Extracting element to D
	output_file_ << "@R13"	<< endl;		// R13: temp. register
	output_file_ << "M=D" << endl;			// R13 = element

	// TEMP segment: fixed base at 0x5.
	if (segment == "temp") {
		output_file_ << "@R5" << endl;			// A = 0x5
		output_file_ << "D=A" << endl;
		output_file_ << "@" << index << endl; 	// A = offset
		output_file_ << "D=D+A" << endl;		// D = base+offset		
	}
	// POINTER segment selects between THIS and THAT (no dereferences).
	else if (segment == "pointer") {
		if (index == 0)
			output_file_ << "@THIS" << endl;
		else
			output_file_ << "@THAT" << endl;
		
		output_file_ << "D=A" << endl;
	}
	else if (segment == "static") {
		output_file_ << "@" << input_filename_ << "." << index << endl; // xxx.i
		output_file_ << "D=A" << endl;
	}
	// All other segments have variable bases, so we need to dereference them.
	else {
		output_file_ << "@" << segment << endl; // A -> segment base
		output_file_ << "D=M" << endl;			// D = segment base
		output_file_ << "@" << index << endl;	// A = offset
		output_file_ << "D=D+A" << endl;		// D = base+offset
	}
	
	output_file_ << "@R14" << endl;
	output_file_ << "M=D" << endl;			// R14 = base+offset

	output_file_ << "@R13" << endl;
	output_file_ << "D=M" << endl;			// D = element
	output_file_ << "@R14" << endl;
	output_file_ << "A=M" << endl;			// A -> base + index
	output_file_ << "M=D" << endl;

	output_file_ << "@SP" << endl;			// SP--
	output_file_ << "M=M-1" << endl;
}

void CodeWriter::writeLabel(const std::string& label) {
	output_file_ << "(";
	/*if (!functionName.empty())
		output_file_ << functionName << "$"; */

	output_file_ << label << ")" << std::endl;
}

void CodeWriter::writeGoto(const std::string& destination) {
	output_file_ << "@" << destination << std::endl;
	output_file_ << "0;JMP" << std::endl;
}

void CodeWriter::writeIf(const std::string& destination) {
	using namespace std;
	output_file_ << "@SP" << endl;		// Pop element to D
	output_file_ << "M=M-1" << endl;	// SP--
	output_file_ << "A=M" << endl;		// A -> if guard
	output_file_ << "D=M" << endl;		// D = if guard
	output_file_ << "@" << destination << endl;	// A -> destination
	output_file_ << "D;JNE" << endl;	// Jump to s if guard != 0
}

void CodeWriter::writePushRegister(const std::string& origin) {
// Useful to push registers while saving contexts.	
	using namespace std;
	output_file_ << "@" << origin << endl;
	output_file_ << "D=M" << endl;		// D = value to push
	output_file_ << "@SP" << endl;
	output_file_ << "A=M" << endl;		// A -> top of stack
	output_file_ << "M=D" << endl;		// Pushing element
	output_file_ << "@SP" << endl;		// SP++
	output_file_ << "M=M+1" << endl;
}

void CodeWriter::writePopRegister(const std::string& destination) {
// Useful to push registers while saving contexts.	
	using namespace std;
	output_file_ << "@SP" << endl;
	output_file_ << "A=A-1" << endl;
	output_file_ << "D=M" << endl;		// D = element
	output_file_ << "@" << destination << endl;
	output_file_ << "M=D" << endl;		// Dest. = element
	output_file_ << "@SP" << endl;
	output_file_ << "M=M-1" << endl;	// SP--
}


void CodeWriter::writeCall(const std::string& name, unsigned int args) {
	using namespace std;
	// Pushing return address:
	unsigned int labelId = label_counter_++;
	// retAddressLabel = return-addressId
	std::string retAddressLabel("retPoint");
	retAddressLabel.append(name);
	retAddressLabel.append(std::to_string(labelId));

	output_file_ << "@" << retAddressLabel << endl;
	output_file_ << "D=A" << endl;		// D = return address
	output_file_ << "@SP" << endl;
	output_file_ << "A=M" << endl;		// A -> top of stack
	output_file_ << "M=D" << endl;		// Pushing element
	output_file_ << "@SP" << endl;		// SP++
	output_file_ << "M=M+1" << endl;

	writePushRegister("LCL");
	writePushRegister("ARG");
	writePushRegister("THIS");
	writePushRegister("THAT");

	// ARG = SP-args-5
	output_file_ << "@SP" << endl;
	output_file_ << "D=M" << endl;			// D = SP
	output_file_ << "@" << args << endl;	// A = args
	output_file_ << "D=D-A" << endl;		// D = SP-args
	output_file_ << "@5" << endl;
	output_file_ << "D=D-A" << endl;		// D = SP-args-5
	output_file_ << "@ARG" << endl;
	output_file_ << "M=D" << endl;			// ARG = SP-args-5

	// LCL = SP
	output_file_ << "@SP" << endl;
	output_file_ << "D=M" << endl;			// D = SP
	output_file_ << "@LCL" << endl;
	output_file_ << "M=D" << endl;			// LCL = SP

	writeGoto(name);
	writeLabel(retAddressLabel);
}

void CodeWriter::writeReturn() {
	using namespace std;
	output_file_ << "@LCL" << endl;
	output_file_ << "D=M" << endl;		// D = LCL
	output_file_ << "@R15" << endl;		// Temp. var. FRAME
	output_file_ << "M=D" << endl;		// FRAME = LCL

	output_file_ << "@5" << endl;
	output_file_ << "A=D-A" << endl;	// A -> FRAME-5 
	output_file_ << "D=M" << endl;		// D = [FRAME-5]
	output_file_ << "@R16" << endl;		// Temp. var. RET
	output_file_ << "M=D" << endl;		// RET = FRAME-5

	writePop("ARG", 0);					// Result into arg[0] for the caller.

	output_file_ << "@ARG" << endl;
	output_file_ << "D=M+1" << endl;	// D = ARG+1
	output_file_ << "@SP" << endl;
	output_file_ << "M=D" << endl;		// SP = ARG+1 (SP restored)

	// Restoring registers:
	const std::string registers[4] = {"THAT", "THIS", "ARG", "LCL"};
	for (unsigned int i = 0; i < 4; i++) {
		output_file_ << "@R15" << endl;
		output_file_ << "D=M" << endl;		// D = FRAME
		output_file_ << "A=D-1" << endl;	// A-> frame-1
		output_file_ << "D=M" << endl;		// D = [frame-1]
		output_file_ << "@" << registers[i] << endl;
		output_file_ << "M=D" << endl;		// THAT = [frame-1]
		output_file_ << "@R15" << endl;		// FRAME--
		output_file_ << "M=M-1" << endl;
	}

	// Goto RET.
	output_file_ << "@R16" << endl;
	output_file_ << "A=M" << endl;			// A -> RET address
	output_file_ << "0;JMP" << endl;

}

void CodeWriter::writeFunction(const std::string& name, unsigned int args) {
	using namespace std;
	writeLabel(name);
	while (args > 0) {
		writePush("constant", 0);
		args--;
	}
}