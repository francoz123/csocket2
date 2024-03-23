/**
 * C program to demonstrate Socket Server.
 * Creates a socket and listens to the supplied port for connection
 * and responds based commands sent from the client.
 * Usage: 
 *	Compile: make
 * 	Run: ./server <port>
 * 
 * @author: Francis Ozoka - 220228986 
*/

#include "protocol.h"
#include "database.h"
#include "aes/aes.c"

int main(int argc, char* argv[])
{
	if (argc != 2) { // Ensures port number is supplied
		printf("Usage: %s <IP address> <port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int PORT = atoi(argv[1]); 
	// Socket variables
	int server_fd, client_fd; 
	socket_server_address server_address;
	socklen_t server_address_length = sizeof(server_address);

	int n, compose_flag, num_msg = -1;
	char buffer[BUFFER_SIZE] = { 0 };
	struct AES_ctx ctx;
	char key[] = "7R5dPbNj!#h@a2Fk";
	// Initialize ctx
    AES_init_ctx(&ctx, (uint8_t*) key);
	// Buffers
	char username[256], password[256], command[26], read_cmd[4] = "READ", compose_cmd[] = "COMPOSE";
	message_node_t *head, *tail, *cursur;
	head = tail = cursur = 0;
    server_fd = create_socket();
    bind_socket(server_fd, &server_address, PORT); // Bind port

	if (listen(server_fd, 3) < 0) {// Listen for connections 
		perror("listen error");
		exit(EXIT_FAILURE);
	}

	printf("Waiting connection...\n");
	// Accept connections with client_fd
	if ((client_fd = accept(server_fd, (socket_address_ptr)&server_address, &server_address_length)) < 0) {
		perror("Failed to establish connection.");
		exit(EXIT_FAILURE);
	}

	printf("Connection established.\n");
	ssize_t read_count;
	auth_token_t auth;
    
	if ((read_count= read(client_fd, &auth, sizeof(auth_token_t))) < 0) { // Read slient data
		perror("Read error");
		exit(EXIT_FAILURE);
	} 
	
	// Decrypt username and password
    AES_ECB_decrypt(&ctx, (uint8_t*)auth.username);
    AES_ECB_decrypt(&ctx, (uint8_t*)auth.password);
	
	strcpy(username, auth.username);
    strcpy(password, auth.password); 
	//fprintf(stderr, "%s\n%s", username, password);
	// Make sure user exists
	while (!find_user(username, password)) {
		if ((read_count = send(client_fd, &num_msg, sizeof(num_msg), 0)) == -1) {
			perror("Send error\n");
			exit(EXIT_FAILURE);
		}

		if ((read_count= read(client_fd, &auth, sizeof(auth_token_t))) < 0) { // Read slient data
			perror("Read error");
			exit(EXIT_FAILURE);
		} 
		// Decrypt username and password
		AES_ECB_decrypt(&ctx, (uint8_t*)auth.username);
		AES_ECB_decrypt(&ctx, (uint8_t*)auth.password);
		
		strcpy(username, auth.username);
		strcpy(password, auth.password);  
	}
	// Populate linked list of messages. Return numbers of messages for the user
	num_msg = create_database(&head, &tail, username); 
	// Send result to the client
	if ((read_count = send(client_fd, &num_msg, sizeof(num_msg), 0)) == -1) {
		perror("Send error\n");
		exit(EXIT_FAILURE);
	}

    const char* exit_str = "EXIT";
	char recipient[256], *rest;

    while (1) {
		//Reset input and command buffers
		memset(buffer, '\0', BUFFER_SIZE);
		memset(command, '\0', sizeof(command));

        if ((read_count = read(client_fd, buffer, BUFFER_SIZE)) < 0) {
            perror("Read error from server");
			save_database(&head); // Save messages
            exit(EXIT_FAILURE);
        } 
		//buffer[strlen(buffer)-1] = 0;
		buffer[strcspn(buffer, "\n\r")] = 0;
		sscanf(buffer, "%s %n", command, &n);
		rest  = buffer + n;
        if (strcmp(buffer, exit_str) == 0) break;
		// READ command action - retrieve next message for user if available
		if (strncmp(command, read_cmd, sizeof(read_cmd)) == 0 && strlen(rest) == 0) {
			printf("READ CMD\n");
			memset(buffer, '\0', BUFFER_SIZE);
			if ((n = read_next_message(&head, &cursur, username, buffer)) == 1) {
				if ((read_count = send(client_fd, buffer, BUFFER_SIZE, 0)) == -1) {
					perror("Send error\n");
					exit(EXIT_FAILURE);
				}
				remove_node(&head, &tail, &cursur);
				/* char sender[256], message[512];
				sscanf(buffer, "%s %s", sender, message);
				sender[strlen(sender)] = 0;
				strcat(buffer, " read your message - ");
				strcat(buffer, message);

				if (!save_message(sender, username, buffer, &head, &tail)) { // Ensure meaage was sent
					printf("Save error");
					exit(EXIT_FAILURE);
				} */
			} else { // No message
				if ((read_count = send(client_fd, "READ ERROR", sizeof("READ ERROR"), 0)) == -1) {
					perror("Send error\n");
					exit(EXIT_FAILURE);
				}
			}
			compose_flag = 0; // Toggle compose flage off
			continue;
		} else if(strcmp(command, compose_cmd) == 0) {
			strcpy(recipient, rest);// Set recipient name
			compose_flag = 1; // Set flag to expect none command message
			continue;
		} 

		if (compose_flag == 1) {// Expecting message for the recipient
			printf("FLAG CMD--%s\n", command);
			// Save message node
			if (save_message(username, recipient, buffer, &head, &tail)) { // Ensure meaage was sent
				strcpy(buffer, "MESSAGE SENT");
				send(client_fd, buffer, BUFFER_SIZE, 0);
				if ((read_count = send(client_fd, buffer, BUFFER_SIZE, 0)) == -1) {
					perror("Send error\n");
					exit(EXIT_FAILURE);
				}
			}else send(client_fd, "MESSAGE FAILED", sizeof("MESSAGE FAILED"), 0);
		} else { // Bad command received
			if ((read_count = send(client_fd, "ERROR", sizeof("ERROR"), 0)) == -1) {
                perror("Send error\n");
                exit(EXIT_FAILURE);
            }
			break;
		}
    }
    
	save_database(&head);
	// closing the sockets
	close(client_fd);
	close(server_fd);

	return 0;
}
