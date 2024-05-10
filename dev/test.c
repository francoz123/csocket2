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
#include "aes/aes.c"
#include "database.h"
#include "sha256.c"

int main()
{
    char username[256] = "U";
    char password[256];
    char *p = SHA256("Q");

    for (int i = 0; i < 64; i++)
    {
        password[i] = *p;
        p++;
    }
    password[64] = 0;#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h> 
#include "protocol.h"
#include <netdb.h>
#include "aes/aes.c"
#include "database.h"
#include "sha256.c"

int main()
{
    char username[256] = "U";
    char password[256];
    char *p = SHA256("Qghyykk");

    for (int i = 0; i < 64; i++)
    {
        password[i] = *p;
        p++;
    }
    password[64] = 0;
    printf("%s\n", password);
    FILE *fd;
    user_t user;// = {"U",  password};
    strcpy(user.username, username);
    strcpy(user.password, password);
    size_t read_value;

    if ((fd = fopen("users", "a")) == NULL) {
        perror("Failed to open file\n");
        return -1;
    }

    fwrite(&user, sizeof(user_t), 1, fd);
    fclose (fd);
    // Find user

    if ((fd = fopen("users", "r")) == NULL) {
        perror("Failed to open file\n");
        return -1;
    }
    while ((read_value = fread(&user, sizeof(user), 1, fd)) != 0){
        if (read_value < 1) {
            printf("File Read error\n");
            fprintf(stderr,"Read error from load\n");
        }

        if (strcmp(user.username, username)==0 && strcmp(user.username, username)==0){
            printf("success\n"); 
            break;   
        }

    }

    printf("%s\n", user.username);
    fclose(fd);
    return 0;
}
    printf("%s\n", password);
    FILE *fd;
    user_t user = {"U",  password};
    /* strcpy(user.username, username);
    strcpy(user.password, password); */
    size_t read_value;

    if ((fd = fopen("users", "a")) == NULL) {
        perror("Failed to open file\n");
        return -1;
    }

    fwrite(&user, sizeof(user_t), 1, fd);
    fclose (fd);
    // Find user

    if ((fd = fopen("users", "r")) == NULL) {
        perror("Failed to open file\n");
        return -1;
    }
    while ((read_value = fread(&user, sizeof(user), 1, fd)) != 0){
        if (read_value < 1) {
            printf("File Read error\n");
            fprintf(stderr,"Read error from load\n");
        }

        if (strcmp(user.username, username)==0 && strcmp(user.username, username)==0){
            printf("success\n"); 
            break;   
        }

    }

    printf("%s\n", user.password);
    fclose(fd);
    return 0;
}