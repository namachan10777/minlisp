CC := gcc
CFLAGS := -g -Wall -Wextra -std=c11
EXE := minlisp

COMPILER := $(CC) $(CFLAGS)

.PHONY: all

all: build

gc.o: gc.c node.h
	$(COMPILER) -o gc.o -c gc.c -I.

parser.o: parser.c parser.h gc.h
	$(COMPILER) -o parser.o -c parser.c -I.

main.o: main.c gc.h node.h parser.h
	$(COMPILER) -o main.o -c main.c -I.

node.o: node.c node.h
	$(COMPILER) -o node.o -c node.c -I.

build: main.o gc.o parser.o node.o
	$(COMPILER) main.o gc.o parser.o node.o -lm -o $(EXE) 

clean:
	rm -f *.o minlisp
