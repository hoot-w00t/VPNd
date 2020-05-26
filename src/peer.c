#include "interface.h"
#include "attributes.h"
#include "peer.h"
#include "vpnd.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>

static peer_t *peers = NULL;

// allocate a peer
peer_t *create_peer(int s, bool is_client)
{
    peer_t *peer = malloc(sizeof(peer_t));

    if (!peer)
        return NULL;
    peer->s = s;
    peer->is_client = is_client;
    peer->alive = true;
    peer->next = NULL;
    return peer;
}

// free peer
void destroy_peer(peer_t *peer)
{
    free(peer);
}

// append peer to the list
void append_peer(peer_t *peer)
{
    peer_t *last = peers;
    if (!peers) {
        peers = peer;
    } else {
        while (last->next)
            last = last->next;
        last->next = peer;
    }
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

// receive data from remote peer and write it to the tun/tap device
void *peer_receive(void *arg)
{
    peer_t *peer = (peer_t *) arg;
    uint8_t buf[VPND_BUFSIZE];
    ssize_t n = 0;

    while ((n = recv(peer->s, buf, VPND_BUFSIZE, MSG_NOSIGNAL)) > 0) {
        tuntap_write(buf, n);
        broadcast_data_to_peers(buf, n, peer);
    }
    return NULL;
}

// thread to handle a peer connection
void *_peer_connection(void *arg)
{
    peer_t *peer = (peer_t *) arg;
    pthread_t thread_recv;

    printf("Peer connected\n");
    pthread_create(&thread_recv, NULL, peer_receive, peer);
    pthread_join(thread_recv, NULL);
    peer->alive = false;
    close(peer->s);
    printf("Peer disconnected\n");
    return NULL;
}

// handle a peer connection
void peer_connection(int s, bool is_client, bool block)
{
    peer_t *peer = create_peer(s, is_client);
    pthread_t peer_thread;

    if (!peer) {
        fprintf(stderr, "Could not allocate memory for peer\n");
        return;
    }
    append_peer(peer);
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
            send(peer->s, data, n, MSG_NOSIGNAL);
            sent_to += 1;
        }
        peer = peer->next;
    }
    printf("Broadcasted %ld bytes (%s) to %lu %s\n",
            n,
            exclude ? "peer" : "local",
            sent_to,
            sent_to == 1 ? "peer" : "peers");
}

// continuously read from the tun/tap interface and send the read data to all
// connected peers
void *_broadcast_tuntap_device(UNUSED void *arg)
{
    uint8_t buf[VPND_BUFSIZE];
    ssize_t n = 0;

    while ((n = tuntap_read(buf, VPND_BUFSIZE)) > 0) {
        broadcast_data_to_peers(buf, n, NULL);
    }
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