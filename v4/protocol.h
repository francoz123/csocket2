/**
 * ****************** protocol.h **********************
 * Abstracts socket creation functions for code decongestion
 * @author Francis Ozoka - 220228986
*/

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctype.h>

#ifndef PROTOCOL_HEADER
#define PROTOCOL_HEADER
//#define PORT 1234
#define BUFFER_SIZE 1024

typedef struct sockaddr_in socket_server_address;
typedef struct sockaddr* socket_address_ptr;

/**
 * Creates a socket
 * @param void
 * @return int - Socket file descriptor
*/
int create_socket() 
{
	int socket_fd;
    // Create socket
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Failed to create socket.");
		exit(EXIT_FAILURE);
	}
	return socket_fd;
}

/**
 * Binds a socket to a port
 * @param socket_fd Socket file descriptor
 * @param server_address socket_server_address type
 * @param PORT port number to bind to
 * @return void
 */
void bind_socket(int socket_fd, socket_server_address* server_address, int PORT)
{
	server_address->sin_family = AF_INET;
	server_address->sin_addr.s_addr = INADDR_ANY;
	server_address->sin_port = htons(PORT);

	// Bind socket to the port 8080
	if (bind(socket_fd, (socket_address_ptr)server_address, sizeof(*server_address)) < 0) {
		perror("Bind failure");
		exit(EXIT_FAILURE);
	}
}

/**
 * BiConnects a socket to another listening socket
 * @param socket_fd Socket file descriptor
 * @param server_address socket_server_address type
 * @param PORT port number to bind to
 * @return void
 */
void connect_to_server(int socket_fd, socket_server_address* server_address, int PORT)
{
	server_address->sin_family = AF_INET;
	server_address->sin_port = htons(PORT);
	int status;
	if ((status= connect(socket_fd, (socket_address_ptr)server_address, sizeof(*server_address)))
		< 0) {
		perror("Connection failure");
		exit(EXIT_FAILURE);
	}
}

/**
 * Ckecks for space character in a string
 * @param str string to be checked
 * @param size length of the string to be parsed
 * @return inr 1 if poitive, else zero
 */
int contains_space_character(char* str, int size)
{
	for (int i = 0; i < size-1; i++){
		if (isspace(str[i])){
			return 1;
		}
	}
	return 0;
}
#endif