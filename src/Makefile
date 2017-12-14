CC := gcc
CFLAGS := -g -Wall -Wextra -std=c11
EXE := minlisp

COMPILER := $(CC) $(CFLAGS)

.PHONY: all

all: build

gc.o: gc.c node.h
	$(COMPILER) -o gc.o -c gc.c -I.

main.o: main.c gc.h node.h 
	$(COMPILER) -o main.o -c main.c -I.

build: main.o gc.o
	$(COMPILER) main.o gc.o -o $(EXE) 
