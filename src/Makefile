CC := gcc
CFLAGS := -g -Wall -Wextra -std=c11
EXE := minlisp

COMPILER := $(CC) $(CFLAGS)

.PHONY: all

all: build

util.o: util.c util.h
	$(COMPILER) -o util.o -c util.c -I.

gc.o: gc.c node.h util.h
	$(COMPILER) -o gc.o -c gc.c -I.

parser.o: parser.c parser.h gc.h node.h  util.h
	$(COMPILER) -o parser.o -c parser.c -I.

main.o: main.c gc.h node.h parser.h
	$(COMPILER) -o main.o -c main.c -I.

node.o: node.c node.h util.h
	$(COMPILER) -o node.o -c node.c -I.

env.o: env.c env.h gc.h node.h util.h
	$(COMPILER) -o env.o -c env.c -I.

eval.o: eval.c eval.h env.h gc.h node.h util.h
	$(COMPILER) -o eval.o -c eval.c -I.

build: main.o gc.o parser.o node.o env.o eval.o util.o
	$(COMPILER) main.o gc.o parser.o node.o env.o eval.o util.o -lm -o $(EXE) 

clean:
	rm -f *.o minlisp
