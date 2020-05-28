#include "interface.h"
#include "attributes.h"
#include "peer.h"
#include "peer_net.h"
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
    pthread_mutex_init(&peer->mutex, NULL);
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
    pthread_mutex_destroy(&peer->mutex);
    pthread_mutex_init(&peer->mutex, NULL);
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
    if (peer->alive) {
        close(peer->s);
        while (peer->alive) {
            printf("Waiting for thread %s:%u to exit...\n", peer->address, peer->port);
            sleep(1);
        }
    }
    pthread_mutex_destroy(&peer->mutex);
    free(peer->address);
    free(peer);
}

// destroy all peers
void destroy_peers(void)
{
    peer_t *current = peers;
    peer_t *next = NULL;

    peers = NULL;
    if (current)
        printf("Closing all connections...\n");
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

// thread to handle a peer connection
void *_peer_connection(void *arg)
{
    peer_t *peer = (peer_t *) arg;
    pthread_t thread_recv;

    printf("Established connection with %s:%u\n",
           peer->address,
           peer->port);

    broadcast_keepalive();
    pthread_create(&thread_recv, NULL, peer_receive, peer);
    pthread_join(thread_recv, NULL);

    peer->alive = false;
    close(peer->s);

    printf("Lost connection with %s:%u\n",
           peer->address,
           peer->port);

    pthread_exit(NULL);
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

// send n bytes of data to all peers if exclude is NULL
// otherwise do not send to the excluded peer
void broadcast_data_to_peers(uint8_t *data, size_t n, peer_t *exclude)
{
    peer_t *peer = peers;
    size_t sent_to = 0;

    while (peer) {
        if (peer->alive && peer != exclude) {
            send_data_to_peer(data, n, peer);
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
    fprintf(stderr, "Local TUN/TAP packets will no longer be broadcasted to peers\n");
    pthread_exit(NULL);
}

// continuously read from the tun/tap interface and send the read data to all
// connected peers
pthread_t broadcast_tuntap_device(void)
{
    pthread_t thread;

    pthread_create(&thread, NULL, _broadcast_tuntap_device, NULL);
    return thread;
}