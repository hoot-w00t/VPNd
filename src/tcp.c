/*
    VPNd - a VPN daemon
    Copyright (C) 2020  akrocynova

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "vpnd.h"
#include "tcp.h"
#include "logger.h"
#include "peer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

static int tcp4_socket = -1;
static int tcp6_socket = -1;

// accept an incoming TCP connection on a listening socket
int tcp_accept_connection(const int s)
{
    char address[INET6_ADDRSTRLEN];
    uint16_t port = 0;
    socklen_t sin_len = sizeof(struct sockaddr);
    struct sockaddr sin;
    int peer;

    memset(address, 0, sizeof(address));
    if ((peer = accept(s, &sin, &sin_len)) < 0) {
        logger(LOG_ERROR, "Connection failed: %s",
                          strerror(errno));
        return -1;
    }

    if (sin.sa_family == AF_INET6) {
        struct sockaddr_in6 *conn = (struct sockaddr_in6 *) &sin;

        inet_ntop(AF_INET6,
                  conn->sin6_addr.__in6_u.__u6_addr8,
                  address,
                  sizeof(address));

        port = conn->sin6_port;

    } else {
        struct sockaddr_in *conn = (struct sockaddr_in *) &sin;

        inet_ntop(AF_INET,
                  &conn->sin_addr.s_addr,
                  address,
                  sizeof(address));

        port = conn->sin_port;
    }

    peer_connection(address,
                    port,
                    peer,
                    false,
                    false);

    return 0;
}

// close all TCP servers
void tcp_server_close(void)
{
    if (tcp4_socket > 0) {
        logger(LOG_INFO, "Closing TCP4 server socket...");
        close(tcp4_socket);
        tcp4_socket = -1;
    }
    if (tcp6_socket > 0) {
        logger(LOG_INFO, "Closing TCP6 server socket...");
        close(tcp6_socket);
        tcp6_socket = -1;
    }
}

// open TCP listening socket(s)
int tcp_server(const bool ip4, const bool ip6, const int backlog)
{
    if (ip4) {
        tcp4_socket = tcp4_bind(NULL, DEFAULT_PORT, backlog);
        if (tcp4_socket < 0) {
            tcp_server_close();
            return -1;
        }
        logger(LOG_INFO, "TCP4 socket listening...");
    }
    if (ip6) {
        tcp6_socket = tcp6_bind("::1", DEFAULT_PORT, backlog);
        if (tcp6_socket < 0) {
            tcp_server_close();
            return -1;
        }
        logger(LOG_INFO, "TCP6 socket listening...");
    }

    struct pollfd pfds[2];
    pfds[0].fd = tcp4_socket;
    pfds[0].events = POLLIN;
    pfds[1].fd = tcp6_socket;
    pfds[1].events = POLLIN;

    while (tcp4_socket > 0 || tcp6_socket > 0) {
        int events = poll(pfds, 2, 1000);

        if (events < 0) {
            logger(LOG_ERROR, "poll() error: %s",
                              strerror(errno));
            tcp_server_close();
            return -1;
        } else if (events > 0) {
            for (int i = 0; i < 2; ++i) {
                if (pfds[i].revents & POLLIN) {
                    tcp_accept_connection(pfds[i].fd);
                }
            }
        }
    }
    return 0;
}

// create a TCP connection to a remote server
int tcp_client(const char *hostname, uint16_t port)
{
    struct addrinfo *ai = NULL;
    int err;

    if ((err = getaddrinfo(hostname, NULL, NULL, & ai)) != 0) {
        logger(LOG_ERROR, "%s", gai_strerror(err));
        return -1;
    }

    char address[INET6_ADDRSTRLEN];
    struct sockaddr *sin = ai->ai_addr;

    memset(address, 0, sizeof(address));

    if (ai->ai_family == AF_INET6) {
        struct sockaddr_in6 *conn = (struct sockaddr_in6 *) sin;

        inet_ntop(AF_INET6,
                  &conn->sin6_addr.__in6_u.__u6_addr8,
                  address,
                  sizeof(address));

        conn->sin6_port = htons(port);

    } else {
        struct sockaddr_in *conn = (struct sockaddr_in *) sin;

        inet_ntop(AF_INET,
                  &conn->sin_addr,
                  address,
                  sizeof(address));

        conn->sin_port = htons(port);
    }

    // Create socket
    int s = socket(ai->ai_family,
                   SOCK_STREAM,
                   0);

    if (s == -1) {
        logger(LOG_CRIT, "Could not create client socket: %s",
                         strerror(errno));
        freeaddrinfo(ai);
        return -1;
    }

    // Connect to remote host
    logger(LOG_WARN, "Connecting to %s:%u...\n", address, port);
    if (connect(s, sin, sizeof(struct sockaddr_in6)) == -1) {
        logger(LOG_ERROR, "Could not connect to %s:%u: %s",
                            address,
                            port,
                            strerror(errno));
        close(s);
        freeaddrinfo(ai);
        return -1;
    }

    peer_connection(address, port, s, true, true);
    freeaddrinfo(ai);

    return 0;
}