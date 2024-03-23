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

	int n, compose_flag, num_msg;
	char buffer[BUFFER_SIZE] = { 0 };
	// Buffers
	char username[256], command[26], read_cmd[4] = "READ", compose_cmd[] = "COMPOSE";

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
	ssize_t read_value;

	if ((read_value= read(client_fd, buffer, BUFFER_SIZE)) < 0) { // Read slient data
		perror("Read error");
		exit(EXIT_FAILURE);
	} 
	printf("Connection established.\n");

	buffer[strcspn(buffer, "\n\r")] = 0;// Remove carriage return from buffer
	sscanf(buffer, "%s %s", command, username); 
	// Populate linked list of messages. Return numbers of messages for the user
	num_msg = create_database(&head, &tail, username); 
	// Send result to the client
	send(client_fd, &num_msg, sizeof(num_msg), 0);

    const char* exit_str = "EXIT";
	char recipient[256], *rest;

    while (1) {
		//Reset input and command buffers
		memset(buffer, '\0', sizeof(buffer));
		memset(command, 0, sizeof(command));

        if ((read_value = read(client_fd, buffer, BUFFER_SIZE)) < 0) {
            perror("Read error from server");
			save_database(&head); // Save messages
            exit(EXIT_FAILURE);
        } 
		buffer[strcspn(buffer, "\n\r")] = 0;
		sscanf(buffer, "%s %n", command, &n);
		rest  = buffer + n;
        if (strcmp(buffer, exit_str) == 0) break;
		// READ command action - retrieve next message for user if available
		if (strncmp(command, read_cmd, sizeof(read_cmd)) == 0 && strlen(rest) == 0) {
			/* cursur = head;
			while (cursur && strcmp(cursur->message->recipient, username) != 0) {
				cursur = cursur->next;
			}

			if (cursur) {// Copy message details to buffer and send to client
				strcpy(buffer, cursur->message->sender);
				strcat(buffer, ": ");
				strcat(buffer, cursur->message->message);
				send(client_fd, buffer, BUFFER_SIZE, 0);
				remove_node(&head, &tail, &cursur);
			} else send(client_fd, "READ ERROR", sizeof("READ ERROR"), 0); */
			if ((n = read_next_message(&head, &cursur, username, buffer)) == 1) {
				send(client_fd, buffer, BUFFER_SIZE, 0);
				remove_node(&head, &tail, &cursur);
			} else send(client_fd, "READ ERROR", sizeof("READ ERROR"), 0); // No message

			compose_flag = 0; // Toggle compose flage off
			continue;
		} else if(strcmp(command, compose_cmd) == 0) {
			strcpy(recipient, rest);// Set recipient name
			compose_flag = 1; // Set flag to expect none command message
			continue;
		} 

		if (compose_flag == 1) {// Expecting message for the recipient
			// Create and save message node
			/* if (!head) {
				head = tail = create_node();
				head->message = (message_t*)malloc(sizeof(message_t));
				head->next = 0;
				strcpy(head->message->sender, username);
				strcpy(head->message->recipient, recipient);
				strcpy(head->message->message, buffer);
			}else {
				message_node_t *temp = (message_node_t*)malloc(sizeof(message_node_t));
				temp->message = (message_t*)malloc(sizeof(message_t));
				strcpy(temp->message->sender, username);
				strcpy(temp->message->recipient, recipient);
				strcpy(temp->message->message, buffer);
				temp->next = 0;
				tail->next = temp;
				tail = temp;
			} */
			if (save_message(username, recipient, buffer, &head, &tail)) { // Ensure meaage was sent
				strcpy(buffer, "MESSAGE SENT");
				send(client_fd, buffer, BUFFER_SIZE, 0);
			}else send(client_fd, "MESSAGE FAILED", sizeof("MESSAGE FAILED"), 0);
		} else { // Bad command received
			send(client_fd, "ERROR", sizeof("ERROR"), 0);
			break;
		}
    }
    
	save_database(&head);
	// closing the sockets
	close(client_fd);
	close(server_fd);

	return 0;
}
