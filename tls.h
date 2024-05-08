#include <openssl/ssl.h>
#include <openssl/err.h>
#include <arpa/inet.h>
#include <netinet/in.h> 


int create_socket2(int port)
{
    int s;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }

    if (listen(s, 1) < 0) {
        perror("Unable to listen");
        exit(EXIT_FAILURE);
    }

    return s;
}

SSL_CTX *create_context()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx)
{
    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

void configure_context2(SSL_CTX *ctx, char* cert, char* key)
{
    /* Set the key and cert */
   /*  if (SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    } */

    // Set verify_callback function
    //SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, verify_callback);
}


 // Callback function for certificate verification
/*int verify_callback(int preverify_ok, X509_STORE_CTX *ctx) {
     // If preverify_ok is already false, the certificate verification failed
    if (!preverify_ok) {
        return 0;
    }

    // Get the certificate being verified
    X509 *cert = X509_STORE_CTX_get_current_cert(ctx);

    // Get the issuer and subject names from the certificate
    char issuer[256];
    char subject[256];
    X509_NAME_oneline(X509_get_issuer_name(cert), issuer, sizeof(issuer));
    X509_NAME_oneline(X509_get_subject_name(cert), subject, sizeof(subject));

    // Compare issuer and subject for self-signed certificates
    if (strcmp(issuer, subject) == 0) {
        printf("Self-signed certificate verified successfully.\n");
        return 1; // Return 1 to indicate successful verification
    } else {
        printf("Certificate verification failed: Certificate is not self-signed.\n");
        return 0; // Return 0 to indicate verification failure
    } 

    return 1;
}*/
int verify_callback(int preverify_ok, X509_STORE_CTX *ctx) {
    return 1;
}