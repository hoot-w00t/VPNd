#include "interface.h"
#include "attributes.h"
#include "peer.h"
#include "protocol.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

static peer_t *peers = NULL;

// initialize peer with NULL values
void initialize_peer(peer_t *peer)
{
    peer->address = NULL;
    peer->port = 0;
    peer->s = -1;
    peer->is_client = false;
    peer->alive = false;
    peer->next = NULL;
}

// set peer information
void set_peer_info(peer_t *peer, struct sockaddr_in *sin, int s, bool is_client)
{
    free(peer->address);
    peer->address = strdup(inet_ntoa(sin->sin_addr));
    peer->port = sin->sin_port;
    peer->s = s;
    peer->is_client = is_client;
    peer->alive = true;
    peer->next = NULL;
}

// allocate and initialize a peer
peer_t *create_peer(struct sockaddr_in *sin, int s, bool is_client)
{
    peer_t *peer = malloc(sizeof(peer_t));

    if (!peer)
        return NULL;
    initialize_peer(peer);
    set_peer_info(peer, sin, s, is_client);
    return peer;
}

// free peer
void destroy_peer(peer_t *peer)
{
    free(peer->address);
    free(peer);
}

// destroy all peers
void destroy_peers(void)
{
    peer_t *current = peers;
    peer_t *next = NULL;

    while (current) {
        next = current->next;
        destroy_peer(current);
        current = next;
    }
}

// add peer to the list, if all slots are free then allocate one
// if a slot is free (alive == false) it is set to the new peer's
// information
peer_t *add_peer(struct sockaddr_in *sin, int s, bool is_client)
{
    peer_t *last = peers;

    while (last) {
        if (last->alive == false) {
            set_peer_info(last, sin, s, is_client);
            return last;
        }
        if (last->next == NULL) {
            break;
        } else {
            last = last->next;
        }
    }

    peer_t *peer = create_peer(sin, s, is_client);
    if (!peers) {
        peers = peer;
    } else {
        last->next = peer;
    }
    return peer;
}

// decode frame and process information
void decode_frame(uint8_t *buf, size_t n, peer_t *peer)
{
    uint32_t payload_len = get_payload_size(buf);

    if (payload_len != n - FRAME_HEADER_SIZE) {
        fprintf(stderr, "Received invalid payload from %s:%u\n",
                        peer->address,
                        peer->port);
        return;
    }

    switch (*buf) {
        case HEADER_DATA:
            tuntap_write(&buf[FRAME_HEADER_SIZE], payload_len);
            broadcast_data_to_peers(buf, n, peer);
            break;

        case HEADER_KEEPALIVE:
            printf("Keep Alive received from %s:%u\n", peer->address, peer->port);
            break;

        default:
            printf("Invalid header type: %x\n", *buf);
            break;
    }
}

// receive data from remote peer and write it to the tun/tap device
void *peer_receive(void *arg)
{
    peer_t *peer = (peer_t *) arg;
    uint8_t *buf = malloc(sizeof(uint8_t) * FRAME_MAXSIZE);
    ssize_t n = 0;

    if (!buf) {
        fprintf(stderr, "Could not allocate memory for peer\n");
        return NULL;
    }
    while ((n = recv(peer->s, buf, FRAME_MAXSIZE, MSG_NOSIGNAL)) >= (signed) FRAME_HEADER_SIZE) {
        decode_frame(buf, n, peer);
    }
    return NULL;
}

// thread to handle a peer connection
void *_peer_connection(void *arg)
{
    peer_t *peer = (peer_t *) arg;
    pthread_t thread_recv;

    printf("Established connection with %s:%u\n",
           peer->address,
           peer->port);

    pthread_create(&thread_recv, NULL, peer_receive, peer);
    pthread_join(thread_recv, NULL);

    peer->alive = false;
    close(peer->s);

    printf("Lost connection with %s:%u\n",
           peer->address,
           peer->port);

    return NULL;
}

// handle a peer connection
void peer_connection(struct sockaddr_in *sin, int s, bool is_client, bool block)
{
    peer_t *peer = add_peer(sin, s, is_client);
    pthread_t peer_thread;

    if (!peer) {
        fprintf(stderr, "Could not allocate memory for peer\n");
        return;
    }

    pthread_create(&peer_thread, NULL, _peer_connection, peer);

    if (block) {
        pthread_join(peer_thread, NULL);
    } else {
        pthread_detach(peer_thread);
    }
}

void sendall(int s, uint8_t *data, size_t n, int flags)
{
    ssize_t _n = 0;
    size_t sent = 0;

    while (sent < n) {
        _n = send(s, &data[sent], n - sent, flags);
        if (_n <= 0) {
            fprintf(stderr, "Socket error: Could not send all the data\n");
            return;
        }
        sent += _n;
    }
}

// send n bytes of data to all peers if exclude is NULL
// otherwise do not send to the excluded peer
void broadcast_data_to_peers(uint8_t *data, size_t n, peer_t *exclude)
{
    peer_t *peer = peers;
    size_t sent_to = 0;

    while (peer) {
        if (peer->alive && peer != exclude) {
            sendall(peer->s, data, n, MSG_NOSIGNAL);
            sent_to += 1;
        }
        peer = peer->next;
    }
}

// continuously read from the tun/tap interface and send the read data to all
// connected peers
void *_broadcast_tuntap_device(UNUSED void *arg)
{
    uint8_t *buf = malloc(sizeof(uint8_t) * FRAME_MAXSIZE);
    ssize_t n = 0;

    if (!buf) {
        fprintf(stderr, "Could not allocate memory for *databuf\n");
        exit(EXIT_FAILURE);
    }
    while ((n = tuntap_read(&buf[FRAME_HEADER_SIZE], sizeof(uint8_t) * FRAME_MAXSIZE)) > 0) {
        encode_frame(buf, n, HEADER_DATA);
        broadcast_data_to_peers(buf, FRAME_HEADER_SIZE + n, NULL);
    }
    free(buf);
    exit(EXIT_FAILURE);
    return NULL;
}

// continuously read from the tun/tap interface and send the read data to all
// connected peers
void broadcast_tuntap_device(bool block)
{
    pthread_t broadcast_thread;

    pthread_create(&broadcast_thread, NULL, _broadcast_tuntap_device, NULL);
    if (block) {
        pthread_join(broadcast_thread, NULL);
    } else {
        pthread_detach(broadcast_thread);
    }
}