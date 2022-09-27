# Homework 1 Makefile
OBJS = user_methods.c 	# Shared Object file list
CC = gcc		# Designated Compiler
CFLAGS = -Wall -O3 -O 	# Compiler Flags set

# file dependencies
user_methods.o: user_methods.c
head.o: head.c user_methods.h
tail.o: tail.c user_methods.h
findlocationfast.o: findlocationfast.c user_methods.h


# BASH Script
findlocation:
	./findlocation.sh


# C Programs
all: head tail findlocationfast

head: $(OBJS) head.o
	$(CC) $(CFLAGS) $@ $^

tail: $(OBJS) tail.o
	$(CC) $(CFLAGS) $@ $^

findlocationfast: S(OBJS) findlocationfast.o
	$(CC) $(CFLAGS) $@ $^


run-head: head
	./head

run-tail:tail
	./tail

run-fast: findlocationfast
	./findlocationfast

# Command to remove files generated on compilation 
clean:
	rm -f *.o *.c~ *.h~ *~ *.c# *.h#


