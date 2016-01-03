#!/usr/bin/env python3
import sys


def prettifyVM(input, output):
	input_data = input.readlines()

	indent_level = 1
	for line in input_data:
		if ("constructor" in line or "function" in line or 
			"method" in line or "void" in line or "label" in line):
			output.write(line)
		else:
			indentation = '\t' * indent_level
			output.write(indentation + line)


def main():
	if len(sys.argv) < 2:
		input = sys.stdin
	else:
		input = open(sys.argv[1], 'r')

	output = sys.stdout
	prettifyVM(input, output)

	input.close()
	output.close()

if __name__ == '__main__':
	main()