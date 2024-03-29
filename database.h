/**
 * ****************** database.h **********************
 * Abstracts types fro creating and storing messages
 * @author Francis Ozoka - 220228986
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef DATABASE
#define DATABASE

enum message_type {notification, messaage};

typedef struct {
    enum message_type type;
    char sender[256];
    char recipient[256];
    char message[1024];
} message_t;

typedef struct message_node{   
    message_t *message;
    struct message_node *next;
} message_node_t;

typedef struct {
    char username[256];
    char password[256];
} user_t;

/**
 * Fetches messages from a file and populates message linked list
 * @param head pointer to the head pointer
 * @param tail pointer to the tail pointer
 * @param username 
 * @return int number of messages, -1 for failure
 */
int create_database(message_node_t **head, message_node_t **tail, char *username)
{
    FILE *fd;
    message_t message;
    size_t read_value;
    int n = 0;

    if ((fd = fopen("database", "r")) == NULL) {
        perror("Failed to open file\n");
        return -1;
    }
    // Read messages and store in linked list
    while ((read_value = fread(&message, sizeof(message_t), 1, fd)) != 0){
        if (read_value < 1) {
            printf("File Read error\n");
        }

        if (!strcmp(message.recipient, username)) n++; // count users messages

        if (!(*head)) {
            *head = *tail = (message_node_t*)malloc(sizeof(message_node_t));
            (*head)->message = (message_t*)malloc(sizeof(message_t));
            strcpy((*head)->message->sender, message.sender);
            strcpy((*head)->message->recipient, message.recipient);
            strcpy((*head)->message->message, message.message);
            (*head)->next = NULL;
            continue;
        }

        message_node_t *temp = (message_node_t*)malloc(sizeof(message_node_t));
        temp->message = (message_t*)malloc(sizeof(message_t));
        strcpy(temp->message->sender, message.sender);
        strcpy(temp->message->recipient, message.recipient);
        strcpy(temp->message->message, message.message);
        temp->next = 0;
        (*tail)->next = temp;
        (*tail) = temp;
        temp = 0;
    }

    fclose(fd);
    return n;
}

/**
 * Fetches message for the user from message queue
 * @param head pointer to the head pointer
 * @param tail pointer to the tail pointer
 * @param cursur pointer to temporary pointer for fetching messages
 * @param username
 * @param buffer Storage got message information
 * @return int 1 for success, -1 for failure
 */
int read_next_message(message_node_t **head, message_node_t **cursur, char *username, char *buffer) 
{
    *cursur = *head;
    while (*cursur && strcmp((*cursur)->message->recipient, username) != 0) {
        *cursur = (*cursur)->next;
    }

    if (*cursur) {// Copy message details to buffer and send to client
        strcpy(buffer, (*cursur)->message->sender);
        strcat(buffer, ": ");
        strcat(buffer, (*cursur)->message->message);
        return 1;
    }else return -1;
}

/**
 * Allocates memory and returns a message node pointer
*/
message_node_t* create_node(){
    message_node_t *node = (message_node_t*)malloc(sizeof(message_node_t));
    node->message = (message_t*) malloc(sizeof(message_t));
    node->next = 0;
    return node;
}

/**
 * Fetches message for the user from message queue
 * @param head pointer to the head pointer
 * @param username
 * @return int 1 number of messages
 */
int count_messages(message_node_t **head, char *username)
{
    message_node_t *temp = *head;
    int n = 0;

    while (temp) {
        if (strcmp(temp->message->recipient, username) == 0) {
            n++;
        }
        temp = temp->next;
    }

    return n;
}

/**
 * Fetches message for the user from message queue
 * @param head pointer to the head pointer
 * @return int 1 for success, -1 for failure
 */
int save_database(message_node_t **head)
{
    FILE *fd;
    message_node_t *prev, *temp = *head;

    if ((fd = fopen("database", "w")) == NULL) {
        perror("Failed to open file\n");
        return -1;
    }

    int n;

    while (temp) {
        if ((n = fwrite(temp->message, sizeof(message_t), 1, fd) < 1)) {
            perror("Write failed");
            return -1;
        }
        prev = temp;
        temp = temp->next;
        free(prev->message);
        free(prev);
    }

    fclose(fd);
    return 1;
}

/**
 * Removes a message node from message queue
 * @param head pointer to the head pointer
 * @param tail pointer to the tail pointer
 * @param cursur pointer to message node to be removed
 * @return void
 */
void remove_node(message_node_t **head, message_node_t** tail, message_node_t **cursur) {
    
    if (*head == *tail && strcmp((*head)->message->recipient, (*cursur)->message->recipient) == 0) {
        free((*head)->message);
        free(*head);
        *head = *tail = *cursur = 0;
        return;
    }
    message_node_t *prev, *temp;
    if (strcmp((*head)->message->recipient, (*cursur)->message->recipient) == 0) {
        temp = *head;
        *head = (*head)->next;
        temp->next = 0;
        free(temp->message);
        free(temp);
        *cursur = 0;
        return;
    }

    prev = *head;
    temp = (*head)->next;
    while (temp && strcmp(temp->message->recipient, (*cursur)->message->recipient) != 0) {
        prev = temp;
        temp = temp->next;
    }
    prev->next = temp->next;
    if (temp == *tail) *tail = prev;
    free(temp->message);
    free(temp);
    cursur = 0;
}

/**
 * Saves message to the database (Linked list)
 * @param sender char* sender
 * @param recipient char* recipient of the message
 * @param buffer message to be stored
 * @param head pointer to the head pointer
 * @param tail pointer to the tail pointer
 * @return void
 */
int save_message(char* sender, char* recipient, char buffer[], message_node_t** head, message_node_t **tail){
    enum message_type mt = strcmp(sender, "NOTIFICATION") == 0 ? notification : messaage;
    if (!(*head)) {
        *head = *tail = create_node();
        (*head)->message = (message_t*)malloc(sizeof(message_t));
        (*head)->next = 0;
        strcpy((*head)->message->sender, sender);
        strcpy((*head)->message->recipient, recipient);
        strcpy((*head)->message->message, buffer);
        (*head)->message->type = mt;
        return 1;
    }else {
        message_node_t *temp = create_node();
        temp->message = (message_t*)malloc(sizeof(message_t));
        strcpy(temp->message->sender, sender);
        strcpy(temp->message->recipient, recipient);
        strcpy(temp->message->message, buffer);
        temp->message->type = mt;
        temp->next = 0;
        (*tail)->next = temp;
        (*tail) = temp;
        return 1;
    }
    return -1;
}

/**
 * Attempta to dfind a user from the database 
 * @param username char* 
 * @param password char* 
 * @param opt int: 0 matches username only, 1 matches username and password
 * @return int: 1 on success, 0 on failure
 */
int find_user(char *username, char *password, int opt)
{
    FILE *fd;
    user_t user;
    size_t read_value;

    if ((fd = fopen("users", "r")) == NULL) {
        perror("Failed to open file\n");
        exit(EXIT_FAILURE);
    }
    // Find user
    while ((read_value = fread(&user, sizeof(user_t), 1, fd)) != 0){
        if (read_value < 1) {
            printf("File Read error\n");
        }

        if (opt && !strcmp(user.username, username) && !strcmp(user.password, password)) return 1; 
        else if (!strcmp(user.username, username)) return 1; 
    }

    fclose(fd);
    return 0;
}

/**
 * Adds a new user to the database
 * @param username char* 
 * @param password char* 
 * @return void
 */
void add_user(char *username, char *password)
{
    FILE *fd;
    user_t user;
    strcpy(user.username, username);
    strcpy(user.password, password);

    if ((fd = fopen("users", "a")) == NULL) {
        perror("Failed to open file\n");
        exit(EXIT_FAILURE);
    }

    if (fwrite(&user, sizeof(user_t), 1, fd) < 0) {
        perror("Wrote error\n");
        exit(EXIT_FAILURE);
    }

    fclose (fd);
}
#endif