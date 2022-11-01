# Homework 2 Makefile
CC = gcc			# Designated Compiler
CFLAGS = -Wall -g -O0    	# Compiler Flags set

# File dependencies
implementation.o: implementation.c
	gcc -fPIC -Wall -g -O0 -c implementation.c 
memory.o: memory.c 
	gcc -fPIC -Wall -g -O0 -c memory.c 

# C Programs
all: implementation memory

implementation:  implementation.o 
	$(CC) $(CFLAGS)  $@ $^

memory: memory.o
	$(CC) $(CFLAGS) $@ $^

run-impl: implementation
	./implementation

run-mem: memory 
	./memory

# Command to remove files generated on compilation 
clean:
	rm -f *.o *.c~ *.h~ *~ *.c# *.h# memory.so
