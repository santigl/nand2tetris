#!/bin/bash
# Nand2Tetris Project
# SantiX -- 15/12/2014
# Deletes all .out files.

# Checking arguments.
if [ $# -eq 0 ]; then 	
	echo "Error: You need to specify a directory."
	echo "Usage: $0 directory"
	echo "Deletes *all* .out files recursively."
	exit -1
fi


echo "Cleaning all .tst files in $1..."
for t in `find $1 -name *.out`; do 
	rm -v $t
done
