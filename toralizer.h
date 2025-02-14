// toralizer.h
#ifndef TORALIZER_H
#define TORALIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>

#define PROXY "127.0.0.1"
#define PROXYPORT 9050
#define SOCKS_VERSION 4
#define CONNECT_COMMAND 1
#define BUFFER_SIZE 8192

typedef unsigned char int8;
typedef unsigned short int int16;
typedef unsigned int int32;

// SOCKS4 request structure
struct proxy_request {
    int8 vn;        // SOCKS version number
    int8 cd;        // Command code
    int16 dstport;  // Destination port
    int32 dstip;    // Destination IP
    unsigned char userid[8];  // User ID field
};
typedef struct proxy_request Req;

// SOCKS4 response structure
struct proxy_response {
    int8 vn;        // Version number
    int8 cd;        // Response code
    int16 dstport;  // Destination port (ignored)
    int32 dstip;    // Destination IP (ignored)
};
typedef struct proxy_response Res;

// Function prototypes
int create_connection(const char *host, int port);
int send_proxy_request(int sockfd, const char *host, int port);
int receive_proxy_response(int sockfd);
void handle_error(const char *msg);
int resolve_host(const char *host, struct in_addr *addr);
void cleanup_socket(int sockfd);

#endif // TORALIZER_H