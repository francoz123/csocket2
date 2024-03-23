#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h> 
//#include "protocol.h"
#include <netdb.h>
#include "aes/aes.c"
//#include "database.h"

int main()
{

    // Client-side code
/* int totalSent = 0;
while (totalSent < bufferSize) {
    int bytesSent = write(sockfd, buffer + totalSent, bufferSize - totalSent);
    if (bytesSent < 0)
        error("ERROR writing to socket");
    totalSent += bytesSent;
}

// Server-side code
int totalReceived = 0;
while (totalReceived < bufferSize) {
    int bytesRead = read(newsockfd, buffer + totalReceived, bufferSize - totalReceived);
    if (bytesRead < 0)
        error("ERROR reading from socket");
    totalReceived += bytesRead;
} */

char key[] = "7R5dPbNj!#h@a2Fk";
// Define the plaintext (must be a multiple of 16 bytes)
char plaintext[] = "PlainText";
struct AES_ctx ctx; 
/* for (uint8_t i = 0; i <= 16; i++)
{
    ctx.Iv[i] = i;
}
for (uint8_t i = 0; i < 177; i++)
{
    ctx.RoundKey[i] = i;
} */
AES_init_ctx(&ctx, (uint8_t*) key);

printf("Plaintext:\n");

char buf[1024];
printf("\n");
//strcpy(ct,pt
AES_ECB_encrypt(&ctx, (uint8_t*)plaintext);
strcpy(buf, plaintext); char t[256], t2[256];
strcat(buf, " ");
strcat(buf, plaintext);
sscanf(buf, "%s %s", t, t2);
printf("Ciphertext:\n");
printf("%s ", buf);
printf("\n");
AES_ECB_decrypt(&ctx, (uint8_t*)t);
AES_ECB_decrypt(&ctx, (uint8_t*)t2);
printf("Plaintext:\n");
printf("%s\n", t);
printf("%s ", t2);
//strcpy(ct,pt);
printf("finished");
}