#
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
#
# for C++ define  CC = g++
CC=gcc
CFLAGS=-g -Wall
FILE=tree

# typing 'make' will invoke the first target entry in the file 
# (in this case the default target entry)
# you can name this target entry anything, but "default" or "all"
# are the most commonly used names by convention
#

default:	clean build

  

myprog.o:	myprog.c
	$(CC) $(CFLAGS) -c myprog.c
 
myprog.o:	myprog.c
	$(CC) $(CFLAGS) -c myprog.c
 
build:	myprog.o
	ar rcs mythread.a myprog.o 

clean:
	rm -rf *.a *.o *.out

