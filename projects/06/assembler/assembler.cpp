#include "assembler.h"

Assembler::Assembler(const std::string& in): 
input_filename_(in),
// Address from which variables can be stored. Starts at 16.
stack_pointer_(0x0010),
// Predefined symbols:
symbol_table_({ 
	{"SP", 		0x0},
	{"LCL", 	0x1},
	{"ARG",  	0x2},
	{"THIS", 	0x3},
	{"THAT", 	0x4},
	{"R0", 		0x0},
	{"R1", 		0x1},
	{"R2", 		0x2},
	{"R3", 		0x3},
	{"R4", 		0x4},
	{"R5", 		0x5},
	{"R6", 		0x6},
	{"R7", 		0x7},
	{"R8", 		0x8},
	{"R9", 		0x9},
	{"R10", 	0xA},
	{"R11", 	0xB},
	{"R12", 	0xC},
	{"R13", 	0xD},
	{"R14", 	0xE},
	{"R15", 	0xF},
	{"SCREEN",  0x4000},
	{"KBD",		0x6000}
})

{
// FIRST PASS: Scanning line by line looking for L_COMMANDS (labels in the code).
	Parser p1(in);
	unsigned int word = 0;	
	// Parser::currentLine() returns the absolute line number in the file,
	// so it is not useful here.
	while (p1.hasMoreCommands()) {	
		p1.advance();
		if (p1.commandType() == L_COMMAND) { 	// Label definition.
			symbol_table_[p1.symbol()] = word;
		}
		else {		// If it is a regular command, count the memory location.
			word++; // (Labels aren't stored in RAM, so they don't count.)
		}
	}
// SECOND PASS: Scanning line by line looking for variable declarations.
	Parser p2(in);
	while (p2.hasMoreCommands()) {
		p2.advance();
		if (p2.commandType() == A_COMMAND) {
			std::string s = p2.symbol();
			if (!isdigit(s.front()) && !symbol_table_.count(s)) {
				symbol_table_[s] = stack_pointer_++; // Var. declaration.
			}
		}
	}
}

void Assembler::translate(std::ostream& out) const {
// PRE: The symbol table has been populated by the constructor.
	Parser p(input_filename_);
	Code c;

	while (p.hasMoreCommands()) {
		p.advance();
		switch (p.commandType()) {
			case A_COMMAND: 
			{
				std::string s = p.symbol();		// immediate or a memory position 
				std::bitset<16> addr;			// defined by a symbol.
				if (isdigit(s.front())) { 		// Starts with a number => immediate.
					addr = c.immediate(s);
				}
				else {							// Reference to a variable.
					assert(symbol_table_.count(s));
					addr = symbol_table_.at(s);	
				}

				out << std::bitset<16>(addr) << std::endl;
				break;
			}
			case C_COMMAND: 
			{
				std::bitset<16> dest(c.dest(p.dest())); // All the codes are stored
				std::bitset<16> jump(c.jump(p.jump())); // with an appropriate padding
				std::bitset<16> comp(c.comp(p.comp())); // to OR them and call it done.
				std::bitset<16> instr(comp | dest | jump);
				out << instr << std::endl;
				break;
			}
			case L_COMMAND:
				break;
		}
	}
}