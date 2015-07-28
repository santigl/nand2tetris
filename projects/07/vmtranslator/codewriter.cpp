#include "codewriter.h"

CodeWriter::CodeWriter(const std::string& out): label_counter_(0) {
	output_file_.open(out, std::ofstream::out);
	assert(output_file_.is_open());
	/*output_file_ << "@2047" << std::endl;
	output_file_ << "D=A" << std::endl;
	output_file_ << "@SP" << std::endl;
	output_file_ << "M=D" << std::endl; // Setting the stack pointer to the base.*/
}

void CodeWriter::setFileName(const std::string& in){
	input_filename_ = in;
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