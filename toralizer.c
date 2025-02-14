// toralizer.c
#include "toralizer.h"

int create_connection(const char *host, int port) {
    int sockfd;
    struct sockaddr_in proxy_addr;
    
    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        handle_error("Failed to create socket");
        return -1;
    }
    
    // Set up proxy address
    memset(&proxy_addr, 0, sizeof(proxy_addr));
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_port = htons(PROXYPORT);
    if (inet_pton(AF_INET, PROXY, &proxy_addr.sin_addr) <= 0) {
        handle_error("Invalid proxy address");
        cleanup_socket(sockfd);
        return -1;
    }
    
    // Connect to proxy
    if (connect(sockfd, (struct sockaddr*)&proxy_addr, sizeof(proxy_addr)) < 0) {
        handle_error("Failed to connect to proxy");
        cleanup_socket(sockfd);
        return -1;
    }
    
    return sockfd;
}

int send_proxy_request(int sockfd, const char *host, int port) {
    Req request;
    struct in_addr addr;
    
    // Initialize request structure
    memset(&request, 0, sizeof(request));
    request.vn = SOCKS_VERSION;
    request.cd = CONNECT_COMMAND;
    request.dstport = htons(port);
    
    // Resolve hostname and set destination IP
    if (resolve_host(host, &addr) < 0) {
        handle_error("Failed to resolve hostname");
        return -1;
    }
    request.dstip = addr.s_addr;
    
    // Send request to proxy
    if (send(sockfd, &request, sizeof(request), 0) != sizeof(request)) {
        handle_error("Failed to send request to proxy");
        return -1;
    }
    
    return 0;
}

int receive_proxy_response(int sockfd) {
    Res response;
    
    // Receive response from proxy
    if (recv(sockfd, &response, sizeof(response), 0) != sizeof(response)) {
        handle_error("Failed to receive proxy response");
        return -1;
    }
    
    // Check response
    if (response.vn != 0 || response.cd != 90) {
        fprintf(stderr, "Proxy request failed with code: %d\n", response.cd);
        return -1;
    }
    
    return 0;
}

int resolve_host(const char *host, struct in_addr *addr) {
    struct hostent *hp;
    
    if (inet_aton(host, addr) != 0) {
        return 0;  // Successfully converted numeric IP
    }
    
    hp = gethostbyname(host);
    if (!hp) {
        return -1;
    }
    
    memcpy(addr, hp->h_addr_list[0], hp->h_length);
    return 0;
}

void handle_error(const char *msg) {
    fprintf(stderr, "Error: %s - %s\n", msg, strerror(errno));
}

void cleanup_socket(int sockfd) {
    if (sockfd >= 0) {
        close(sockfd);
    }
}

int main(int argc, char *argv[]) {
    char *host;
    int port, sockfd;
    
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
        return -1;
    }
    
    host = argv[1];
    port = atoi(argv[2]);
    
    // Create connection to proxy
    sockfd = create_connection(host, port);
    if (sockfd < 0) {
        return -1;
    }
    
    // Send proxy request
    if (send_proxy_request(sockfd, host, port) < 0) {
        cleanup_socket(sockfd);
        return -1;
    }
    
    // Receive and verify proxy response
    if (receive_proxy_response(sockfd) < 0) {
        cleanup_socket(sockfd);
        return -1;
    }
    
    printf("Successfully connected to %s:%d through SOCKS4 proxy\n", host, port);
    
    // At this point, the connection is established and ready for data transfer
    // You can add your data transfer logic here
    
    cleanup_socket(sockfd);
    return 0;
}