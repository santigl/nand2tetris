#include <string>
#include <unordered_map>
#include <cassert>
#include <iostream>

class Code {

public:
	Code();
	uint16_t 	comp(const std::string&) const;
	uint8_t 	dest(const std::string&) const;
	uint8_t 	jump(const std::string&) const;
	uint16_t 	immediate(const std::string&) const;

private:
	std::unordered_map<std::string, uint16_t> opcodes_;
	std::unordered_map<std::string, uint8_t> jumps_;
	std::unordered_map<std::string, uint8_t> dests_;
};