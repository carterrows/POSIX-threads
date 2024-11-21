# Assignment 1 - CIS*3110
# Carter Rows - 1170615
## MAKEFILE for the project

## compile options/flags
CFLAGS = -Wall -g -std=c11

## Library to link
LIB = -lpthread

## executable to build
EXE = A2checker

## define the set of object files we need to build each executable
OBJS = a2main.o messageQueue.o spellCheck.o hashTable.o printing.o

## top level target
all : $(EXE)

## targets for each executable, based on the object files indicated
$(EXE) : $(OBJS)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS) $(LIB)

## remove the results of a build
clean :
	-rm -f $(OBJS) $(EXE)

valgrind :
	valgrind --leak-check=full --fair-sched=yes -s ./$(EXE)

valgrindDRD :
	valgrind --tool=drd -s ./$(EXE)
