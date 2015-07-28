CC=g++
CFLAGS=-std=c++11 -Wall

.PHONY: all clean debug

all: CFLAGS+=-D NDEBUG -O3
all: vmtranslator	

vmtranslator: main.o
	$(CC) $(CFLAGS) parser.o codewriter.o vmtranslator.o main.o -o vmtranslator

parser.o: parser.cpp
	$(CC) $(CFLAGS) -c parser.cpp

codewriter.o: codewriter.cpp parser.o
	$(CC) $(CFLAGS) -c codewriter.cpp

vmtranslator.o: vmtranslator.cpp parser.o codewriter.o	
	$(CC) $(CFLAGS) -c vmtranslator.cpp

main.o: main.cpp parser.o codewriter.o vmtranslator.o
	$(CC) $(CFLAGS) -c main.cpp

clean:
	rm -rf *.o vmtranslator

debug: CFLAGS+=-g
debug: vmtranslator	