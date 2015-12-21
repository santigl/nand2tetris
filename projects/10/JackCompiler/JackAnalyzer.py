#!/usr/bin/env python3
import JackCompilationEngineXML
import sys
import os

def main():
	if len(sys.argv) < 2:
		print("Error. Missing argument: input (dir./file)")
		return -1

	inputPath = sys.argv[1]

	if not os.path.exists(inputPath):
		print("Error:", inputPath, "does not exist.")

	if os.path.isdir(inputPath):
		compileFilesXML(inputPath)
	else:
		compileFileXML(inputPath)


def compileFilesXML(directory):
	''' Get all files in directory that are not .XMLs and compile them. '''
	for f in os.listdir(directory):
		if f.find('xml') == -1:
			compileFileXML(directory + f)

def compileFileXML(inputPath):
	print("Compiling", inputPath, "...", end=' ')
	outFile = outputName(os.path.basename(inputPath))
	JackCompilationEngineXML.JackCompilationEngineXML(inputPath, outFile)
	print("Done")

def outputName(inputFilename):
	''' If the input file has an extension, first remove it. 
	Then append ".XML". '''
	outName = inputFilename
	extension = inputFilename.find('.')
	if extension != -1: # If it has it, strip the .jack extension.
		outName = outName[:extension]
	return outName + ".xml"

if __name__ == '__main__':
	main()