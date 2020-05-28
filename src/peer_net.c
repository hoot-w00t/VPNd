#include "peer.h"
#include "protocol.h"
#include "interface.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>

// make sure that all the data is sent to the socket s
void sendall(int s, uint8_t *data, size_t n, int flags)
{
    ssize_t _n = 0;
    size_t sent = 0;

    while (sent < n) {
        _n = send(s, &data[sent], n - sent, flags);
        if (_n <= 0) {
            fprintf(stderr, "Socket error: Could not send all the data\n");
            return;
        };
        sent += n;
    }
    if (sent != n)
        fprintf(stderr, "Sent %lu bytes but buffer contained %lu bytes\n", sent, n);
}

// data to the given peer if it is alive
void send_data_to_peer(uint8_t *data, size_t n, peer_t *peer)
{
    if (peer->alive) {
        pthread_mutex_lock(&peer->mutex);
        sendall(peer->s, data, n, MSG_NOSIGNAL);
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
ssize_t receive_frame(uint8_t *buf, size_t n, peer_t *peer)
{
    uint32_t payload_len = 0;
    ssize_t m = 0;
    uint32_t data_len = 0;

    while ((m = recv(peer->s, buf, FRAME_HEADER_SIZE, MSG_NOSIGNAL)) != FRAME_HEADER_SIZE) {
        if (m <= 0) {
            return -1;
        } else {
            fprintf(stderr, "Received invalid header of size %i\n", (int) m);
        }
    }
    payload_len = read_uint32(&buf[1]);

    if (payload_len > FRAME_PAYLOAD_MAXSIZE) {
        fprintf(stderr, "Received invalid payload size: %u\n", payload_len);
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
            fprintf(stderr, "Received too much data, %u/%u bytes\n", data_len, payload_len);
            return 0;
        }
    }

    switch (*buf) {
        case HEADER_DATA:
            tuntap_write(&buf[FRAME_HEADER_SIZE], payload_len);
            broadcast_data_to_peers(buf, n, peer);
            break;

        case HEADER_KEEPALIVE:
            printf("Received keep alive from %s:%u\n", peer->address, peer->port);
            break;

        case HEADER_CLOSE:
            printf("Received close from %s:%u\n", peer->address, peer->port);
            return -1;

        default:
            printf("Received invalid header type: %x\n", *buf);
            break;
    }
    return data_len + FRAME_HEADER_SIZE;
}

// receive data from remote peer and write it to the tun/tap device
void *peer_receive(void *arg)
{
    peer_t *peer = (peer_t *) arg;
    uint8_t *buf = malloc(sizeof(uint8_t) * FRAME_MAXSIZE);

    if (!buf) {
        fprintf(stderr, "Could not allocate memory for peer\n");
        return NULL;
    }
    while (receive_frame(buf, FRAME_MAXSIZE, peer) >= 0);
    free(buf);
    pthread_exit(NULL);
}