#include "code.h"

Code::Code(): 
opcodes_({
		// Comp. (a=0)
		{"0",		0b1110101010000000},
		{"1",		0b1110111111000000},
		{"-1", 		0b1110111010000000},
		{"D",		0b1110001100000000},
		{"A",		0b1110110000000000},
		{"!D", 		0b1110001101000000},
		{"!A", 		0b1110110001000000},
		{"-D", 		0b1110001111000000},
		{"-A", 		0b1110110011000000},
		{"D+1", 	0b1110011111000000},
		{"A+1", 	0b1110110111000000},
		{"D-1", 	0b1110001110000000},
		{"A-1", 	0b1110110010000000},
		{"D+A", 	0b1110000010000000},
		{"D-A", 	0b1110010011000000},
		{"A-D", 	0b1110000111000000},
		{"D&A", 	0b1110000000000000},
		{"D|A", 	0b1110010101000000},
		// Comp. (a=1)					
		{"M",		0b1111110000000000},	
		{"!M", 		0b1111110001000000},
		{"-M", 		0b1111110011000000},
		{"M+1", 	0b1111110111000000},
		{"M-1", 	0b1111110010000000},
		{"D+M", 	0b1111000010000000},
		{"D-M", 	0b1111010011000000},
		{"M-D", 	0b1111000111000000},
		{"D&M", 	0b1111000000000000},
		{"D|M", 	0b1111010101000000}	
	}),

	jumps_({
		{"", 		0b000},
		{"JGT", 	0b001},
		{"JEQ", 	0b010},
		{"JGE", 	0b011},
		{"JLT", 	0b100},
		{"JNE", 	0b101},
		{"JLE", 	0b110},
		{"JMP",		0b111}
	}),

	dests_({ 	// d1 d2 d3 0 0 0
		{"", 		0b000000},
		{"M", 		0b001000},
		{"D", 		0b010000},
		{"MD", 		0b011000},
		{"A", 		0b100000},
		{"AM", 		0b101000},
		{"AD", 		0b110000},
		{"AMD",		0b111000}
	})
	/* The padding in dests_ allows to obtain instruction codes by simply
	ORing its parts together. */
{
}


uint16_t Code::comp(const std::string& s) const {
	if (opcodes_.count(s))
		return opcodes_.at(s);
	std::cerr << "Error: '" << s << "' invalid operation." << std::endl;
	throw std::runtime_error("Invalid operation");
}

uint8_t Code::dest(const std::string& s) const {
	if (dests_.count(s))
		return dests_.at(s);
	std::cerr << "Error: '" << s << "' invalid destination operand.";
	std::cerr << std::endl;
	throw std::runtime_error("Invalid destination");
}

uint8_t Code::jump(const std::string& s) const {
	if (jumps_.count(s))
		return jumps_.at(s);
	std::cerr << "Error: '" << s << "' invalid jump mnemonic."; 
	std::cerr << std::endl;
	throw std::runtime_error("Invalid jump");
}

uint16_t Code::immediate(const std::string& s) const {
	int immediate;
	// Attempting to convert string to int:
	try {
		immediate = std::stoi(s);	
	} 
	catch (std::out_of_range) {
		std::cerr << "Error: " << s << " immediate out or range.";
		std::cerr << std::endl;
		throw std::runtime_error("Invalid immediate");
	 }

	// The conversion worked, now checking if it is inside the valid range.
	// The number should fit in 15 bits and be non-negative. (2^15 = 32768).
	if (immediate < 0 || immediate > 32768) {
		std::cerr << "Error: " << s << " immediate out of range.";
		std::cerr << std::endl; 
		throw std::runtime_error("Invalid immediate");
	}
	uint16_t res = immediate;
	return res;
}