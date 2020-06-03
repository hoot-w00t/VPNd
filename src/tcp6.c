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

#include "tcp.h"
#include "logger.h"
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

// create TCP4 socket and bind it to an address and port
// if bind_address is NULL listen on any
// returns the file descriptor or -1 in case of an error
int tcp6_bind(const char *bind_address, const uint16_t bind_port,
    const int backlog)
{
    struct sockaddr_in6 sin;

    // Initialize sin
    memset(&sin, 0, sizeof(struct sockaddr_in6));
    sin.sin6_family = AF_INET6;
    sin.sin6_port = htons(bind_port);
    if (bind_address) {
        if (inet_pton(AF_INET6, bind_address, &sin.sin6_addr) != 1) {
            logger(LOG_ERROR, "Invalid IP address: %s: %s",
                              bind_address,
                              strerror(errno));
            return -1;
        }
    } else {
        sin.sin6_addr = in6addr_any;
    }

    // Create socket
    int s = socket(AF_INET6, SOCK_STREAM, 0);
    if (s < 0) {
        logger(LOG_ERROR, "Could not create server socket: %s",
                          strerror(errno));
        return -1;
    }

    // Avoid binding errors after the socket is closed
    setsockopt(s,
               SOL_SOCKET,
               SO_REUSEADDR,
               &(int) {1},
               sizeof(int));

    // Disable dual-mode
    setsockopt(s,
               IPPROTO_IPV6,
               IPV6_V6ONLY,
               &(int) {1},
               sizeof(int));

    // Bind to address and port
    if (bind(s, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
        logger(LOG_ERROR, "Could not bind to %s:%u: %s",
                          bind_address ? bind_address : "::",
                          bind_port,
                          strerror(errno));
        close(s);
        return -1;
    }

    // Start listening to connections
    if (listen(s, backlog) == -1) {
        logger(LOG_ERROR, "Could not start listening: %s",
                          strerror(errno));
        close(s);
        return -1;
    }

    return s;
}