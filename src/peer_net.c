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

#include "peer.h"
#include "protocol.h"
#include "interface.h"
#include "netroute.h"
#include "packet_header.h"
#include "logger.h"
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>

// send data to *peer if it alive
// this will also ensure that all the data is sent
void send_data_to_peer(uint8_t *data, size_t n, peer_t *peer)
{
    ssize_t _n = 0;
    size_t sent = 0;

    if (peer->alive) {
        pthread_mutex_lock(&peer->mutex);
        while (sent < n) {
            _n = send(peer->s, &data[sent], n - sent, MSG_NOSIGNAL);
            if (_n <= 0) {
                logger(LOG_ERROR, "peer %s:%u: send error: %lu/%lu bytes sent",
                                  peer->address,
                                  peer->port,
                                  sent,
                                  n);
                break;
            };
            sent += n;
        }
        pthread_mutex_unlock(&peer->mutex);
    }
}

// broadcast a keepalive packet to all connected peers
void broadcast_keepalive(void)
{
    uint8_t keepalive[5] = {HEADER_KEEPALIVE, 0, 0, 0, 0};

    broadcast_data_to_peers(keepalive, sizeof(keepalive), NULL);
}

// broadcast a close packet to all connected peers
void broadcast_close(void)
{
    uint8_t brd_close[5] = {HEADER_CLOSE, 0, 0, 0, 0};

    broadcast_data_to_peers(brd_close, sizeof(brd_close), NULL);
}

// decode frame and process information
ssize_t receive_frame(uint8_t *buf, peer_t *peer, netroute_t *route)
{
    ssize_t m = 0;
    uint32_t total = 0;
    uint32_t payload_len = 0;
    peer_t *target = NULL;

    while (total != FRAME_HEADER_SIZE) {
        m = recv(peer->s, &buf[total], FRAME_HEADER_SIZE - total, MSG_NOSIGNAL);
        if (m <= 0) {
            return -1;
        }
        total += m;
    }

    total = 0;
    payload_len = read_uint32(&buf[1]);

    if (payload_len > FRAME_PAYLOAD_MAXSIZE) {
        logger(LOG_ERROR, "peer %s:%u: invalid payload size: %u",
                          peer->address,
                          peer->port,
                          payload_len);
        recv(peer->s, buf, FRAME_MAXSIZE, MSG_NOSIGNAL);
        return 0;
    } else if (payload_len > 0) {
        while (total < payload_len) {
            m = recv(peer->s, &buf[FRAME_HEADER_SIZE + total], payload_len - total, MSG_NOSIGNAL);
            if (m <= 0) {
                return -1;
            } else {
                total += m;
            }
        }
    }

    switch (*buf) {
        case HEADER_DATA:
            packet_destaddr(&buf[FRAME_HEADER_SIZE], route);
            if (is_local_route(route)) {
                packet_srcaddr(&buf[FRAME_HEADER_SIZE], route);
                if (!get_peer_route(route)) {
                    char addr[INET6_ADDRSTRLEN];

                    memset(addr, 0, sizeof(addr));
                    get_netroute_addr(route, addr, sizeof(addr));
                    logger(LOG_DEBUG, "peer %s:%u: adding route: %s",
                                      peer->address,
                                      peer->port,
                                      addr);
                    add_netroute(route, &peer->routes);
                }
                tuntap_write(&buf[FRAME_HEADER_SIZE], payload_len);
            } else if ((target = get_peer_route(route))) {
                send_data_to_peer(buf, total + FRAME_HEADER_SIZE, target);
            } else {
                tuntap_write(&buf[FRAME_HEADER_SIZE], payload_len);
                broadcast_data_to_peers(buf, total + FRAME_HEADER_SIZE, peer);
            }
            break;

        case HEADER_KEEPALIVE:
            logger(LOG_DEBUG, "peer %s:%u: keep alive",
                              peer->address,
                              peer->port);
            break;

        case HEADER_CLOSE:
            logger(LOG_DEBUG, "peer %s:%u: close",
                              peer->address,
                              peer->port);
            return -1;

        default:
            logger(LOG_ERROR, "peer %s:%u: invalid header type: 0x%02x",
                              peer->address,
                              peer->port,
                              *buf);
            break;
    }
    return total + FRAME_HEADER_SIZE;
}

// receive data from remote peer and write it to the tun/tap device
void *peer_receive(void *arg)
{
    peer_t *peer = (peer_t *) arg;
    uint8_t *buf = malloc(sizeof(uint8_t) * FRAME_MAXSIZE);
    netroute_t *route = malloc(sizeof(netroute_t));

    if (!buf || !route) {
        logger(LOG_CRIT, "Could not allocate memory for peer");
        return NULL;
    }

    route->next = NULL;
    route->mac = false;
    route->ip4 = false;

    while (receive_frame(buf, peer, route) >= 0);
    free(buf);
    free(route);
    pthread_exit(NULL);
}