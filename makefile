COMPILER = gcc
CFLAGS = -Wall -pedantic 
EXES = server client
all: ${EXES}

client: protocol.h client.c tls.h
	${COMPILER} ${CFLAGS} client.c -o client -lcurl -lssl -lcrypto -lz

server: database.h protocol.h server.c tls.h
	${COMPILER} ${CFLAGS} server.c -o server -lcurl -lssl -lcrypto -lz 

clean:
	rm -f *.o *~ ${EXES}

	