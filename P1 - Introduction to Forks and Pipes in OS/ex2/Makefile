CC = gcc
CFLAGS += -Wall
all: cypher phrases addmx

cypher: cypher.o
phrases: phrases.o
addmx: addmx.o

cypher.c:
	echo > cypher.c
phrases.c:
	echo > phrases.c
addmx.c:
	echo > addmx.c


clean:
	rm -f cypher
	rm -f phrases
	rm -f addmx
	rm -f *.o

