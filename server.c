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

#include "tls.h"
#include "protocol.h"
#include "database.h"


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

	int n, compose_flag = 0, num_msg = -1;
	char buffer[BUFFER_SIZE] = { 0 };
	char username[256], password[256], command[26], read_cmd[4] = "READ", compose_cmd[] = "COMPOSE";

	message_node_t *head, *tail, *cursur;
	head = tail = cursur = 0;

	/* Initializing OpenSSL */
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();
	
 server_fd = create_socket(PORT);
 /*bind_socket(server_fd, &server_address, PORT); // Bind port

	if (listen(server_fd, 3) < 0) {// Listen for connections 
		perror("listen error");
		exit(EXIT_FAILURE);
	}*/

	printf("Waiting connection...\n");
	// Accept connections with client_fd
	if ((client_fd = accept(server_fd, (socket_address_ptr)&server_address, &server_address_length)) < 0) {
		perror("Failed to establish connection.");
		exit(EXIT_FAILURE);
	}
	printf("Connection established.\n");

 SSL_CTX *ctx = create_context();
 configure_context(ctx);

 SSL *ssl = ssl = SSL_new(ctx);
 SSL_set_fd(ssl, client);

 if (SSL_accept(ssl) <= 0) {
     ERR_print_errors_fp(stderr);
 }
	ssize_t count;
	auth_token_t auth;
	if ((count= read(client_fd, &auth, sizeof(auth_token_t))) < 0) { // Read slient data
		perror("Read error");
		exit(EXIT_FAILURE);
	} 
	    
	strcpy(username, auth.username);
    strcpy(password, auth.password); 

	if (auth.type == signup) {
		if (!find_user(username, password, 0)) add_user(auth.username, auth.password);
			num_msg = -2;

		while (num_msg == -2 && find_user(username, password, 0)) {
			if ((count = SSL_write(ssl, &num_msg, sizeof(num_msg), 0)) == -1) {
				perror("Send error\n");
				exit(EXIT_FAILURE);
			}

			if ((count= read(client_fd, &auth, sizeof(auth_token_t))) < 0) { // Read slient data
				perror("Read error");
				exit(EXIT_FAILURE);
			} 

			strcpy(username, auth.username);
			strcpy(password, auth.password);  
		}
	}
	// Make sure user exists
	while (!find_user(username, password, 1)) {
		if ((count = SSL_write(ssl, &num_msg, sizeof(num_msg), 0)) == -1) {
			perror("Send error\n");
			exit(EXIT_FAILURE);
		}

		if ((count= read(client_fd, &auth, sizeof(auth_token_t))) < 0) { // Read slient data
			perror("Read error");
			exit(EXIT_FAILURE);
		} 

		strcpy(username, auth.username);
		strcpy(password, auth.password);  
	}
	// Populate linked list of messages. Return numbers of messages for the user
	num_msg = create_database(&head, &tail, username); 
	// Send result to the client
	if ((count = SSL_write(ssl, &num_msg, sizeof(num_msg), 0)) == -1) {
		perror("Send error\n");
		exit(EXIT_FAILURE);
	}

    const char* exit_str = "EXIT";
	char recipient[256], *rest;

    while (1) {
		//Reset input and command buffers
		memset(buffer, '\0', sizeof(buffer));
		memset(command, 0, sizeof(command));

        if ((count = read(client_fd, buffer, BUFFER_SIZE)) < 0) {
            perror("Read error from server");
			save_database(&head); // Save messages
            break;
        } 

		buffer[strcspn(buffer, "\n\r")] = 0;
		sscanf(buffer, "%s %n", command, &n);
		rest  = buffer + n;
        if (strcmp(buffer, exit_str) == 0) break;
		// READ command action - retrieve next message for user if available
		if (strncmp(command, read_cmd, sizeof(read_cmd)) == 0 && strlen(rest) == 0) {
			if ((n = read_next_message(&head, &cursur, username, buffer)) == 1) {
				count = SSL_write(ssl, buffer, BUFFER_SIZE, 0);

				if (count < 0) {
					perror("Send error.");
					save_database(&head); // Save messages
					exit(EXIT_FAILURE);
				}

				remove_node(&head, &tail, &cursur);
				char sender[256], msg[512];
				sscanf(buffer, "%s %n", sender, &n);
				strcpy(msg, (buffer + n));
				strcpy(buffer, "[ ");
				strcat(buffer, username);
				strcat(buffer, " read your message: ");
				strcat(buffer, msg);
				strcat(buffer, " ]");
				sender[strlen(sender)-1] = 0; 

				save_message("NOTIFICATION", sender, buffer, &head, &tail);
			} else {
				strcpy(buffer, "READ ERROR");
				count = send(client_fd, buffer, BUFFER_SIZE, 0);

				if (count < 0) {
					perror("Send error.");
					save_database(&head); // Save messages
					exit(EXIT_FAILURE);
				}
			} // No message

			compose_flag = 0; // Toggle compose flage off
			continue;
		} else if(strcmp(command, compose_cmd) == 0) {
			strcpy(recipient, rest);// Set recipient name
			compose_flag = 1; // Set flag to expect none command message
			continue;
		} 

		if (compose_flag == 1) {// Expecting message for the recipient
			if (save_message(username, recipient, buffer, &head, &tail)) { // Ensure meaage was sent
				strcpy(buffer, "MESSAGE SENT");
				count = send(client_fd, buffer, BUFFER_SIZE, 0);

				if (count < 0) {
					perror("Send error.");
					save_database(&head); // Save messages
					exit(EXIT_FAILURE);
				}
			}else {
				count = SSL_write(ssl, "MESSAGE FAILED", sizeof("MESSAGE FAILED"), 0);
				if (count < 0) {
					perror("Send error.");
					save_database(&head); // Save messages
					exit(EXIT_FAILURE);
				}
			}
		} else { // Bad command received
			strcpy(buffer, "ERROR");
			count = SSL_write(ssl, buffer, BUFFER_SIZE, 0);
			if (count < 0) {
				perror("Send error.");
				save_database(&head); // Save messages
				exit(EXIT_FAILURE);
			}
			break;
		}
    }
    
	save_database(&head);
	// closing the sockets
 SSL_shutdown(ssl);
 SSL_free(ssl);
 close(client);
	close(client_fd);
	close(server_fd);
 SSL_CTX_free(ctx);
 EVP_cleanup();

	return 0;
}
