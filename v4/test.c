#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h> 
#include "protocol.h"
#include <netdb.h>

typedef struct {
    char sender[256];
    char recipient[256];
    char message[1024];
} message_t;

typedef struct message_list_node{   
    message_t message;
    struct message_list_node *next;
    struct message_list_node *previous;
} node_t;

node_t *head; 
int main()
{
    char hn[1023]; char *Ipbuff;
    hn[1023] = '\0';
    char hn2[1023];
    hn2[1023] = '\0';
   int j = gethostname(hn, 1023);
   struct hostent *h = gethostbyname("localhost");
   Ipbuff = inet_ntoa(*((struct in_addr*) h->h_addr_list[0]));
   printf("%s\n", hn);
   printf("%s\n", Ipbuff);
}