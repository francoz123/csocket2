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
#include <termios.h> // For terminal settings
#include "protocol.h"
#include "aes/aes.c" //Source: 

void get_password (char *passsword, int size);
void get_username (char *username, int size);

int main(int argc, char const* argv[])
{
    if (argc != 3) {// Ensure hosename and port are supplied
		printf("Usage: %s <IP address> <port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int n, num_msg, client_fd, PORT = atoi(argv[2]); 
    ssize_t count;
	char *host_IP, buffer[1024] = { 0 }, key[] = "7R5dPbNj!#h@a2Fk"; // Encryption key
    const char *host = argv[1]; // Set host name from args
    struct AES_ctx ctx; // Used for aes encryption
    auth_token_t auth; 
    // Program buffers
    char command[256], read_cmd[] = "READ", compose_cmd[] = "COMPOSE";
    
    // Socket variables
    socket_server_address server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);

    // Initialize ctx
    AES_init_ctx(&ctx, (uint8_t*) key);

    printf("Welcome! Please login to interract with the server\n");
    get_username(auth.username, sizeof(auth.username));
    get_password(auth.password, sizeof(auth.password));
    // Encrypt username and password
    AES_ECB_encrypt(&ctx, (uint8_t*)auth.username);
    AES_ECB_encrypt(&ctx, (uint8_t*)auth.password);
    
    // Get host IP
    struct hostent *host_info = gethostbyname(host);
    host_IP = inet_ntoa(*((struct in_addr*) host_info->h_addr_list[0]));
    
    // Initialize socket and connect to server
	client_fd = create_socket(); 
	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, host_IP, &server_address.sin_addr) <= 0) {
		printf("Invalid address/ Address not supported \n");
		return -1;
	}
    connect_to_server(client_fd, &server_address, PORT);
	
    if((count = send(client_fd, &auth, sizeof(auth_token_t), 0)) == -1) {
        perror("Send error\n");
        exit(EXIT_FAILURE);
    }

    if ((count = read(client_fd, &num_msg, sizeof(num_msg))) == -1) {
        perror("Read error\n");
        exit(EXIT_FAILURE);
    }

    while (num_msg == -1) {
        get_username(auth.username, sizeof(auth.username));
        get_password(auth.password, sizeof(auth.password));     
        // Encrypt username and password
        AES_ECB_encrypt(&ctx, (uint8_t*)auth.username);
        AES_ECB_encrypt(&ctx, (uint8_t*)auth.password);

        if((count = send(client_fd, &auth, sizeof(auth_token_t), 0)) == -1) {
            perror("Send error\n");
            exit(EXIT_FAILURE);
        }
        if ((count = read(client_fd, &num_msg, sizeof(num_msg))) == -1) {
            perror("Read error\n");
            exit(EXIT_FAILURE);
        }
    }
    
    printf("- You have %d unread message(s)\n", num_msg);
    char *rest; // input left in buffer after command string

    while (1) {
        memset(buffer, 0, BUFFER_SIZE); //Reset buffer
        printf(">>> ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n\r")] = 0;
        sscanf(buffer, "%s %n", command, &n);
        rest = buffer + n;

        if (strncmp(buffer, "EXIT", 4) == 0) {// Exit on EXIT command
            send(client_fd, "EXIT", sizeof("EXIT"), 0);
            break;
        }
        // READ command
        if (strcmp(command, read_cmd) == 0) {
            while (strcmp(command, read_cmd) == 0 && strcmp(buffer, read_cmd) != 0){
                printf("Invalid READ command. Usage: READ\n");
                printf(">>> ");
                fgets(buffer, BUFFER_SIZE, stdin);
                buffer[strcspn(buffer, "\n\r")] = 0;
                sscanf(buffer, "%s", command);
            }

            if (strcmp(command, read_cmd) == 0) {
                if ((count = send(client_fd, buffer, strlen(buffer), 0)) == -1) {
                    perror("Send error\n");
                    exit(EXIT_FAILURE);
                }
                count = read(client_fd, buffer, BUFFER_SIZE-1); 
                if (count < 0) {
                    printf("Read error\n");
                    exit(EXIT_FAILURE);
                } else if (count == 0){
                    printf("Connection closed by server.\n");
                } else printf("- %s\n", buffer);
            }
        }
        if (strncmp(command, compose_cmd, strlen(compose_cmd)) == 0) {
            while (strcmp(compose_cmd, command) == 0  && (strcspn(rest, " ") != strlen(rest) || rest[0] == '\0')) {
                printf("Invalid command or username contain space(s). Usage: COMPOSE <username>\n");
                printf(">>> ");
                fgets(buffer, BUFFER_SIZE, stdin);
                sscanf(buffer, "%s %n", command, &n);
                buffer[strcspn(buffer, "\n\r")] = 0;
                rest = buffer + n;
            }
            if ((count = send(client_fd, buffer, strlen(buffer), 0)) == -1) {
                perror("Send error\n");
                exit(EXIT_FAILURE);
            }
        } else {// Probable message
            if ((count = send(client_fd, buffer, strlen(buffer), 0)) == -1) {
                perror("Send error\n");
                exit(EXIT_FAILURE);
            }
            count = read(client_fd, buffer, BUFFER_SIZE-1); 
            if (count < 0) {
                printf("Read error\n");
                exit(EXIT_FAILURE);
            } else if (count == 0){
                printf("Connection closed by server.\n");
                exit(EXIT_FAILURE);
            } else printf("- %s\n", buffer);
        }
        
    }
	// closing the connected socket
	close(client_fd);
	return 0;
}

void get_username(char *username, int size)
{
    printf("Enter your username: ");
    if (fgets(username, size, stdin) == NULL) {
        printf("Failed to read usename");
    }
    username[strcspn(username, "\n\r")] = 0; // Remove carriage return
    // Ensures username does not contain space
    while (strcspn(username, " ") != strlen(username) || username[0] == '\0') {
        printf("Invalid username. Username must not contain spaces.\n");
        printf("Enter your username: ");
        
        if (fgets(username, size, stdin) == NULL) {
            printf("Failed to read password");
        } 
        username[strcspn(username, "\n\r")] = 0;
    }
}

void get_password(char *password, int size) 
{
    // Structs for terminal settings
    static struct termios old_terminal_settings;
    static struct termios new_terminal_settings;
    // Retrieve old settings
    tcgetattr(STDIN_FILENO, &old_terminal_settings);
    new_terminal_settings = old_terminal_settings;
    // turn off echo
    new_terminal_settings.c_lflag &= ~(ICANON | ECHO);
    new_terminal_settings.c_cc[VTIME] = 0;
    new_terminal_settings.c_cc[VMIN] = 1;
    // Set terminal option to no echo
    tcsetattr(STDIN_FILENO, TCSANOW, &new_terminal_settings);
    // Get password from input
    printf("Enter your password: ");

    if (fgets(password, size, stdin) == NULL) {
        printf("Failed to read password");
    }
    password[strcspn(password, "\n\r")] = 0;

    // Ensures password does not contain space
    while (strcspn(password, " ") != strlen(password) || password[0] == '\0') {
        printf("Invalid sizeof(password). Username must not contain spaces.\n");
        printf("Enter your password: ");

        if (fgets(password, size, stdin) == NULL) {
            printf("Failed to read password");
        } 
        password[strcspn(password, "\n\r")] = 0;
    }
    // Retrun terminal to old settings
    tcsetattr(STDIN_FILENO, TCSANOW, &old_terminal_settings);
    printf("\n");
}