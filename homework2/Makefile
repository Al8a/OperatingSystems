# Homework 2 Makefile
CC = gcc			# Designated Compiler
CFLAGS = -fPIC -Wall -g -O0   	# Compiler Flags set

# File dependencies
implementation.o: implementation.c 
memory.o: memory.c
main.o: main.c

# C Programs
all: implementation memory test

implementation:  implementation.o
	$(CC) $(CFLAGS) $@ $^

memory: memory.o
	$(CC) $(CFLAGS) $@ $^

main:	main.o 
	$(CC) $(CFLAGS) $@ $^



run-impl: implementation
	./implementation

run-mem: memory 
	./memory

run-main: main memory implementation
	./main

# Command to remove files generated on compilation 
clean:
	rm -f *.o *.c~ *.h~ *~ *.c# *.h# test main memory.so
