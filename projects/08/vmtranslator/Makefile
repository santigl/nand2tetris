CC=g++
CFLAGS=-std=c++11 -Wall

.PHONY: all clean debug

all: CFLAGS+=-D NDEBUG -O3
all: VMTranslator	

VMTranslator: main.o
	$(CC) $(CFLAGS) Parser.o CodeWriter.o VMTranslator.o main.o -o VMTranslator

Parser.o: Parser.cpp
	$(CC) $(CFLAGS) -c Parser.cpp

CodeWriter.o: CodeWriter.cpp Parser.o
	$(CC) $(CFLAGS) -c CodeWriter.cpp

VMTranslator.o: VMTranslator.cpp Parser.o CodeWriter.o	
	$(CC) $(CFLAGS) -c VMTranslator.cpp

main.o: main.cpp Parser.o CodeWriter.o VMTranslator.o
	$(CC) $(CFLAGS) -c main.cpp

clean:
	rm -rf *.o VMTranslator

debug: CFLAGS+=-g
debug: VMTranslator	