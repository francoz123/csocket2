/**
 * C program to demonstrate Socket client.
 * Creates a socket and connects to a listening socket
 * sends and receivesmessages from the server
 * Usage: 
 *	Compile: make
 * 	Run: ./client <host name> <port>
 * 
 * @author: Francis Ozoka - 220228986 
*/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h> 
#include <netdb.h>
#include "protocol.h"

int main(int argc, char const* argv[])
{
    if (argc != 3) {// Ensure hosename and port are supplied
		printf("Usage: %s <IP address> <port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int n, num_msg, client_fd, PORT = atoi(argv[2]); 
    ssize_t read_count;
	char buffer[1024] = { 0 };
    const char *host = argv[1]; // Set host name from args
    char *host_IP;
    // Program buffers
    char command[256], username[256], read_cmd[4] = "READ", compose_cmd[] = "COMPOSE";
    // Socket variables
    socket_server_address server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);

    printf("Welcome! Please login to interract with the server\n");
    printf("Enter your username: ");
    // Get username from imput
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "/n/r")] = 0; // Remove carriage return
    // Get host IP
    struct hostent *host_info = gethostbyname(host);
    host_IP = inet_ntoa(*((struct in_addr*) host_info->h_addr_list[0]));
    // Ensures username does not contain spacer
    while (strcspn(username, " ") != strlen(username) || username[0] == '\0') {
        printf("Invalid username. Username must not contain spaces.\n");
        printf("Enter your username: ");
        fgets(username, sizeof(username), stdin);
    }
    // Initialize socket and connect to server
	client_fd = create_socket(); 
	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, host_IP, &server_address.sin_addr) <= 0) {
		printf("Invalid address/ Address not supported \n");
		return -1;
	}
    connect_to_server(client_fd, &server_address, PORT);
	
    printf("CONNECTED: You are ready to communicate with the server.\n\n");
	strcpy(buffer, "LOGIN ");
    strcat(buffer, username);
    send(client_fd, buffer, strlen(buffer), 0);
    read(client_fd, &num_msg, sizeof(num_msg));
    
    num_msg > 0? printf("- You have: %d unread message(s)\n", num_msg): printf("You have no unread messages\n");

    char *rest; // input left in buffer after command string

    while (1) {
        memset(buffer, 0, sizeof(buffer)); //Reset buffer
        printf(">>> ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "/n/r")] = 0;
        sscanf(buffer, "%s %n", command, &n);
        rest = buffer + n;

        if (strncmp(buffer, "EXIT", 4) == 0) {// Exit on EXIT command
            send(client_fd, "EXIT", sizeof("EXIT"), 0);
            break;
        }
        // READ command
        if (strncmp(command, read_cmd, strlen(read_cmd)) == 0) {
            while (strcmp(command, read_cmd) == 0 && strcmp(buffer, read_cmd) != 0){
                printf("Invalid READ command. Usage: READ\n");
                printf(">>> ");
                fgets(buffer, sizeof(buffer), stdin);
                sscanf(buffer, "%s", command);
            }

            if (strcmp(command, read_cmd) == 0) {
                send(client_fd, buffer, strlen(buffer), 0);
                read_count = read(client_fd, buffer, BUFFER_SIZE); 
                if (read_count < 0) {
                    printf("Read error\n");
                    exit(EXIT_FAILURE);
                } else if (read_count == 0){
                    printf("Connection closed by server.\n");
                } else printf("- %s\n", buffer);
            }
        }
        if ((strncmp(command, compose_cmd, strlen(compose_cmd)) == 0)) {
            while (strcmp(compose_cmd, command) == 0  && (strcspn(rest, " ") != strlen(rest) || rest[0] == '\0')) {
                printf("Invalid command or username contain space(s). Usage: COMPOSE <username>\n");
                printf(">>> ");
                fgets(buffer, BUFFER_SIZE, stdin);
                sscanf(buffer, "%s %n", command, &n);
                buffer[strcspn(buffer, "/n/r")] = 0;
                rest = buffer + n;
            }
            send(client_fd, buffer, BUFFER_SIZE, 0);
        } else {// Probable message
            send(client_fd, buffer, BUFFER_SIZE, 0);
            read_count = read(client_fd, buffer, BUFFER_SIZE); 
            if (read_count < 0) {
                printf("Read error\n");
                exit(EXIT_FAILURE);
            } else if (read_count == 0){
                printf("Connection closed by server.\n");
                exit(EXIT_FAILURE);
            } else printf("- %s\n", buffer);
        }
        
    }

	// closing the connected socket
	close(client_fd);
	return 0;
}
