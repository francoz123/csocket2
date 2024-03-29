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
#include <ctype.h>
#include "protocol.h"
#include <termios.h> // For terminal settings
#include "protocol.h"
#include "aes/aes.c" //Source: 

void get_password (char *passsword, int size);
void get_password2(char *password, int size, int opt);
void get_username (char *username, int size);
void get_user_info(char *password, char *username, int size, auth_token_t** auth);
int get_choice();
int strong_password(char *password);

int main(int argc, char const* argv[])
{
    if (argc != 3) {// Ensure hosename and port are supplied
		printf("Usage: %s <IP address> <port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int n, num_msg, client_fd, PORT = atoi(argv[2]); 
    ssize_t count;
	char buffer[1024] = { 0 }, key[] = "7R5dPbNj!#h@a2Fk"; // Encryption key
    const char *host = argv[1]; // Set host name from args
    char *host_IP;
    struct AES_ctx ctx; // Used for aes encryption
    auth_token_t auth; 
    // Program buffers
    char command[256], read_cmd[4] = "READ", compose_cmd[] = "COMPOSE";
    // Socket variables
    socket_server_address server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);

    // Initialize ctx
    AES_init_ctx(&ctx, (uint8_t*) key);
    auth_token_t *auth_ptr = &auth; // Holds username and password
    printf("Welcome! Please login or register to interract with the server\n");
    get_user_info(auth.username, auth.password, sizeof(auth.username), &auth_ptr);
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

    while (num_msg == -2) { // While username exits 
        printf("Username already exists.\n");
        get_user_info(auth.username, auth.password, sizeof(auth.username), &auth_ptr);
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

    while (num_msg == -1) {
        printf("Login failed\n");
        /* get_username(auth.username, sizeof(auth.username));
        get_password(auth.password, sizeof(auth.password));  */    
        get_user_info(auth.username, auth.password, sizeof(auth.username), &auth_ptr);
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
    
    printf("\n\nLogin successful\n");
    printf("- You have %d unread message(s)\n", num_msg);

    char *rest; // input left in buffer after command string

    while (1) {
        memset(buffer, 0, sizeof(buffer)); //Reset buffer
        printf(">>> ");
        fgets(buffer, sizeof(buffer), stdin); // Get input from stdin
        buffer[strcspn(buffer, "\n\r")] = 0;
        sscanf(buffer, "%s %n", command, &n); // 
        rest = buffer + n;

        if (strncmp(buffer, "EXIT", 4) == 0) {// Exit on EXIT command
            AES_ECB_encrypt(&ctx, (uint8_t*)buffer); // Encrypte buffer
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
                AES_ECB_encrypt(&ctx, (uint8_t*)buffer); // Encrypte buffer

                if (send(client_fd, buffer, strlen(buffer), 0) < 0) {
                    perror("Send error\n");
				    exit(EXIT_FAILURE);
                }
                count = read(client_fd, buffer, BUFFER_SIZE); 
                AES_ECB_decrypt(&ctx, (uint8_t*)buffer);

                if (count < 0) {
                    printf("Read error\n");
                    exit(EXIT_FAILURE);
                } else if (count == 0){
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
                buffer[strcspn(buffer, "\n\r")] = 0;
                rest = buffer + n;
            }
            AES_ECB_encrypt(&ctx, (uint8_t*)buffer);

            if (send(client_fd, buffer, strlen(buffer), 0) < 0) {
                perror("Send error\n");
                exit(EXIT_FAILURE);
            }
        } else {// Probable message
            AES_ECB_encrypt(&ctx, (uint8_t*)buffer);

            if (send(client_fd, buffer, BUFFER_SIZE, 0) < 0) {
                perror("Send error\n");
                exit(EXIT_FAILURE);
            }
            count = read(client_fd, buffer, BUFFER_SIZE);
            AES_ECB_decrypt(&ctx, (uint8_t*)buffer);

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

/**
 * Gets username from input
 * @param username
 * @param size int: length of username buffer
*/
void get_username(char *username, int size)
{
    printf("Enter your username: ");

    if (fgets(username, size, stdin) == NULL) {
        printf("Failed to read usename");
    }
    username[size - 1] = 0;
    username[strcspn(username, "\n\r")] = 0; // Remove carriage return
    // Ensures username does not contain space
    while (strcspn(username, " ") != strlen(username) || username[0] == '\0') {
        printf("Invalid username. Username must not contain spaces or be empty.\n");
        printf("Enter your username: ");
        
        if (fgets(username, size, stdin) == NULL) {
            printf("Failed to read password");
        } 
        username[size - 1] = 0;
        username[strcspn(username, "\n\r")] = 0;
    }
    //printf("\n");
}

/**
 * Gets password from input
 * @param password
 * @param size int: length of username buffer
*/
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
    printf("Create password. Password must be between 8 to 50 chaacters, and must contain at least \none"
    " upper case character, one lower case character, one digit, and one special character.\n\n");
    printf("Enter password: ");

    if (fgets(password, size, stdin) == NULL) {
        printf("Failed to read password");
        exit(EXIT_FAILURE);
    }
    password[size - 1] = 0;
    password[strcspn(password, "\n\r")] = 0;

    // Ensures password does not contain space
    while (strcspn(password, " ") != strlen(password) || password[0] == '\0' || strlen(password) < 8
            || !strong_password(password)) {
        if (strcspn(password, " ") != strlen(password)) 
            printf("\n\nInvalid input. Password must not contain spaces.\n");
        else if (password[0] == '\0') printf("\n\nInvalid input. Password can not be empty.\n");
        else if (strlen(password) < 8 || strlen(password) > 50) 
            printf("\n\nInvalid input. Password must be between 8 to 50 chaacters.\n");
        else if (!strong_password(password))  
            printf("\n\nInvalid input. Password must contain at least one upper case character,"
            " one lower case character, one digit, and one special character.\n");

        printf("Enter password: ");

        if (fgets(password, size, stdin) == NULL) {
            printf("Failed to read password");
        } 

        password[size - 1] = 0;
        password[strcspn(password, "\n\r")] = 0;
    }
    // Retrun terminal to old settings
    tcsetattr(STDIN_FILENO, TCSANOW, &old_terminal_settings);
    printf("\n");
}

/**
 * Gets password from input
 * @param password
 * @param size int: length of username buffer
 * @param opt int: 0 for password entry, 1 to confirm password
*/
void get_password2(char *password, int size, int opt) 
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
    if (opt == 1) printf("Confirm password: ");
    else if (opt == 0) printf("Enter password: ");

    if (fgets(password, size, stdin) == NULL) {
        printf("Failed to read password");
        exit(EXIT_FAILURE);
    }

    password[size - 1] = 0;
    password[strcspn(password, "\n\r")] = 0;
    // Retrun terminal to old settings
    tcsetattr(STDIN_FILENO, TCSANOW, &old_terminal_settings);
    printf("\n");
}

/**
 * Gets user info from input for authentication
 * @param password
 * @param username
 * @param size int: length of username and password buffers
 * @param auth authentication_token tobe set
*/
void get_user_info(char *username, char *password, int size,  auth_token_t** auth)
{
    int opt;
    int return_to_options = 0;
    while (!return_to_options) {
        opt  = get_choice();
        if (opt == 1) {
            printf("Enter q to return to options.\n");
            get_username(username, size);
            if (strcmp(username, "q") == 0) continue;
            get_password2(password, size, 0);
            if (strcmp(password, "q") == 0) continue;
            (*auth)->type = login;
        } 

        if (opt == 2) {
            char cp[256];
            printf("Enter q to return to options.\n");
            get_username(username, size);
            if (strcmp(username, "q") == 0) continue;
            get_password(password, size);
            if (strcmp(password, "q") == 0) continue;
            get_password2(cp, size, 1);
            if (strcmp(cp, "q") == 0) continue;
            while (strcmp(password, cp) != 0){
                printf("Passwords do not match.\n\n");
                get_password(password, size);
                if (strcmp(password, "q") == 0) {
                    return_to_options =1;
                    break;
                }
                get_password2(cp, size, 1);
                if (strcmp(cp, "q") == 0) {
                    return_to_options =1;
                    break;
                }

                if (strcmp(cp, "q") == 0) continue;
            }
            (*auth)->type = signup;
        } 
        return_to_options = !return_to_options;
    }
    
}

/**
 * Gets user choice to either login or register
*/
int get_choice()
{
    int opt = 0;
    char input[256], err_msg[256] = "Invalid option. Try again\n\n";
    char user_msg[256] = "Select option by typing the option number and pressing enter\n";
    char *msg = user_msg;
    printf("%s", user_msg);
    printf("1. Login\n2. Register\n3. Exit\n\nEnter option: ");
    fgets(input, 256, stdin);
    sscanf(input,"%d", &opt);
    
    if (opt == 3) exit(EXIT_SUCCESS);

    while (opt != 1 && opt != 2) {
        msg = err_msg;
        printf("%s", msg);
        printf("1. Login\n2. Register\n3. Exit\nEnter option: ");
        scanf("%d", &opt);
    }

    printf("\n");
    return opt;
}

/**
 * Checkes passord strength
 * @param password
 * @return int: 1 if password meets requirements, 0 if not.
*/
int strong_password(char *password) {
    char *p = password;
    int upper, lower, digit, special;
    upper = lower = digit = special = 0;
    char c;

    while (*p) {
        c = *p++;
        if (isupper(c)) upper++;
        else if (islower(c)) lower++;
        else if (isdigit(c)) digit++;
        else if (ispunct(c)) special++;
    }

    return upper && lower && digit && special;
}