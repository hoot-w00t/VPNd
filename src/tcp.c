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
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        fprintf(stderr, "Could not create server socket: %s\n", strerror(errno));
        return -1;
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)); // set reuseaddr option

    // Bind to address and port
    if (bind(s, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
        fprintf(stderr, "Could not bind to %s:%u: %s\n", bind_address, bind_port, strerror(errno));
        close(s);
        return -1;
    }

    // Start listening to connections
    if (listen(s, backlog) == -1) {
        fprintf(stderr, "Could not start listening: %s\n", strerror(errno));
        close(s);
        return -1;
    }

    return s;
}

// accept an incoming TCP connection on a listening socket
int tcp_accept_connection(int s)
{
    struct sockaddr sin;
    socklen_t sin_len = sizeof(struct sockaddr);
    int peer = accept(s, &sin, &sin_len);

    if (peer == -1) {
        fprintf(stderr, "Connection failed: %s\n", strerror(errno));
        return -1;
    }
    peer_connection(peer, false, false);
    return 0;
}

// create a TCP server and accept incoming connections
int tcp_server(const char *bind_address, uint16_t bind_port)
{
    int s = tcp_bind(bind_address, bind_port, 1);

    if (s == -1)
        return -1;

    printf("Listening for incoming connections...\n");
    while (true)
        tcp_accept_connection(s);

    printf("Closing server...\n");
    close(s);
    return 0;
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

    printf("Connected to %s:%u\n", address, port);

    peer_connection(s, true, true);
    return 0;
}