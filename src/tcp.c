#include "tcp.h"
#include "peer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

static int server_socket = -1;

// create socket and bind it to an address and port
// returns the file descriptor or -1 in case of an error
int tcp_bind(const char *bind_address, uint16_t bind_port, int backlog)
{
    struct sockaddr_in sin;

    // Initialize sin
    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(bind_port);
    sin.sin_addr.s_addr = inet_addr(bind_address);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        fprintf(stderr, "Could not create server socket: %s\n", strerror(errno));
        return -1;
    }
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)); // set reuseaddr option

    // Bind to address and port
    if (bind(server_socket, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
        fprintf(stderr, "Could not bind to %s:%u: %s\n", bind_address, bind_port, strerror(errno));
        close(server_socket);
        server_socket = -1;
        return -1;
    }

    // Start listening to connections
    if (listen(server_socket, backlog) == -1) {
        fprintf(stderr, "Could not start listening: %s\n", strerror(errno));
        close(server_socket);
        server_socket = -1;
        return -1;
    }

    return server_socket;
}

// accept an incoming TCP connection on a listening socket
int tcp_accept_connection(void)
{
    struct sockaddr sin;
    socklen_t sin_len = sizeof(struct sockaddr);
    int peer = -1;

    if (server_socket < 0)
        return -1;

    peer = accept(server_socket, &sin, &sin_len);

    if (peer == -1) {
        fprintf(stderr, "Connection failed: %s\n", strerror(errno));
        return -1;
    }

    peer_connection((struct sockaddr_in *) &sin, peer, false, false);
    return 0;
}

// create a TCP server and accept incoming connections
int tcp_server(const char *bind_address, uint16_t bind_port, int backlog)
{
    if (tcp_bind(bind_address, bind_port, backlog) == -1)
        return -1;

    printf("Listening for incoming connections...\n");
    while (server_socket > 0)
        tcp_accept_connection();

    tcp_server_close();
    return 0;
}

// close a TCP server
void tcp_server_close(void)
{
    if (server_socket > 0) {
        printf("Closing server...\n");
        close(server_socket);
        server_socket = -1;
    }
}

// create a TCP connection to a remote server
int tcp_client(const char *address, uint16_t port)
{
    struct sockaddr_in sin;
    in_addr_t _address = 0;

    // Initialize peer_addr
    memset(&sin, 0, sizeof(sin));
    if (inet_pton(AF_INET, address, &_address) != 1) {
        fprintf(stderr, "Invalid IP address: %s\n", address);
        return -1;
    }
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(address);

    // Create socket
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        fprintf(stderr, "Could not create client socket: %s\n", strerror(errno));
        return -1;
    }

    // Connect to remote host
    printf("Connecting to %s:%u...\n", address, port);
    if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
        fprintf(stderr, "Could not connect to %s:%u: %s\n", address, port, strerror(errno));
        close(s);
        return -1;
    }

    peer_connection(&sin, s, true, true);
    return 0;
}