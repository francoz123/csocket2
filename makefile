COMPILER = gcc
CFLAGS = -Wall -pedantic 
EXES = server client
all: ${EXES}

client: protocol.h client.c
	${COMPILER} ${CFLAGS} client.c -o client

server: database.h protocol.h server.c
	${COMPILER} ${CFLAGS} server.c -o server

clean:
	rm -f *.o *~ ${EXES}

	