CC=g++
CFLAGS=-std=c++11 -Wall

.PHONY: all clean debug

all: CFLAGS+=-D NDEBUG -O3
all: assembler	

assembler: main.o
	$(CC) $(CFLAGS) parser.o code.o assembler.o main.o -o assembler

parser.o: parser.cpp
	$(CC) $(CFLAGS) -c parser.cpp

code.o: code.cpp parser.o
	$(CC) $(CFLAGS) -c code.cpp

assembler.o: code.o parser.o assembler.cpp
	$(CC) $(CFLAGS) -c assembler.cpp

main.o: main.cpp parser.o code.o assembler.o
	$(CC) $(CFLAGS) -c main.cpp

clean:
	rm -rf *.o assembler

debug: CFLAGS+=-g
debug: assembler	