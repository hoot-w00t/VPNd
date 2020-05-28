#include "peer.h"
#include "protocol.h"
#include "interface.h"
#include "netroute.h"
#include "packet_header.h"
#include <sys/socket.h>
#include <stdio.h>
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
                fprintf(stderr, "(peer %s:%u) send error, %lu/%lu bytes sent\n",
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
    uint32_t payload_len = 0;
    ssize_t m = 0;
    uint32_t data_len = 0;
    peer_t *target = NULL;

    while ((m = recv(peer->s, buf, FRAME_HEADER_SIZE, MSG_NOSIGNAL)) != FRAME_HEADER_SIZE) {
        if (m <= 0) {
            return -1;
        } else {
            fprintf(stderr, "(peer %s:%u) invalid header of size %i\n",
                    peer->address,
                    peer->port,
                    (int) m);
        }
    }
    payload_len = read_uint32(&buf[1]);

    if (payload_len > FRAME_PAYLOAD_MAXSIZE) {
        fprintf(stderr, "(peer %s:%u) invalid payload size: %u\n",
                peer->address,
                peer->port,
                payload_len);
        recv(peer->s, buf, FRAME_MAXSIZE, MSG_NOSIGNAL);
        return 0;
    } else if (payload_len > 0) {
        while (data_len < payload_len) {
            m = recv(peer->s, &buf[FRAME_HEADER_SIZE + data_len], payload_len - data_len, MSG_NOSIGNAL);
            if (m <= 0) {
                return -1;
            } else {
                data_len += m;
            }
        }
        if (data_len > payload_len) {
            fprintf(stderr, "(peer %s:%u) too much data, %u/%u bytes\n",
                    peer->address,
                    peer->port,
                    data_len,
                    payload_len);
            return 0;
        }
    }

    switch (*buf) {
        case HEADER_DATA:
            packet_destaddr(&buf[FRAME_HEADER_SIZE], route);
            if (is_local_route(route)) {
                packet_srcaddr(&buf[FRAME_HEADER_SIZE], route);
                if (!get_peer_route(route)) {
                    printf("(peer %s:%u) ", peer->address, peer->port);
                    add_netroute(route, &peer->routes);
                }
                tuntap_write(&buf[FRAME_HEADER_SIZE], payload_len);
            } else if ((target = get_peer_route(route))) {
                send_data_to_peer(buf, data_len + FRAME_HEADER_SIZE, target);
            } else {
                tuntap_write(&buf[FRAME_HEADER_SIZE], payload_len);
                broadcast_data_to_peers(buf, data_len + FRAME_HEADER_SIZE, peer);
            }
            break;

        case HEADER_KEEPALIVE:
            printf("(peer %s:%u) keep alive\n", peer->address, peer->port);
            break;

        case HEADER_CLOSE:
            printf("(peer %s:%u) close\n", peer->address, peer->port);
            return -1;

        default:
            printf("(peer %s:%u) invalid header type 0x%02x\n", peer->address, peer->port, *buf);
            break;
    }
    return data_len + FRAME_HEADER_SIZE;
}

// receive data from remote peer and write it to the tun/tap device
void *peer_receive(void *arg)
{
    peer_t *peer = (peer_t *) arg;
    uint8_t *buf = malloc(sizeof(uint8_t) * FRAME_MAXSIZE);
    netroute_t *route = malloc(sizeof(netroute_t));

    if (!buf || !route) {
        fprintf(stderr, "Could not allocate memory for peer\n");
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