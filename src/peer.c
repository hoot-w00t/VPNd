#include "interface.h"
#include "attributes.h"
#include "peer.h"
#include "peer_net.h"
#include "protocol.h"
#include "netroute.h"
#include "packet_header.h"
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
static netroute_t *local_routes = NULL;

// initialize peer with NULL values
void initialize_peer(peer_t *peer)
{
    pthread_mutex_init(&peer->mutex, NULL);
    peer->address = NULL;
    peer->port = 0;
    peer->s = -1;
    peer->is_client = false;
    peer->alive = false;
    peer->routes = NULL;
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
    destroy_netroutes(peer->routes);
    peer->routes = NULL;
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
        for (uint8_t i = 0; i < 10 && peer->alive; ++i) {
            printf("Waiting for thread %s:%u to exit...\n", peer->address, peer->port);
            sleep(1);
        }
        if (peer->alive) {
            printf("Thread %s:%u takes too long to exit\n", peer->address, peer->port);
        }
    }
    pthread_mutex_destroy(&peer->mutex);
    destroy_netroutes(peer->routes);
    free(peer->address);
    free(peer);
}

// destroy all peers
void destroy_peers(void)
{
    peer_t *current = peers;
    peer_t *next = NULL;
    netroute_t *_local_routes = local_routes;

    peers = NULL;
    local_routes = NULL;
    destroy_netroutes(_local_routes);
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

// dump connected peers
void dump_peers(void)
{
    peer_t *peer = peers;

    printf("Connected peers:\n");
    while (peer) {
        printf("    Peer %s:%u (%s)\n", peer->address, peer->port, peer->alive ? "alive" : "dead");
        printf("        Routes:\n");

        netroute_t *route = peer->routes;
        while (route) {
            printf("            ");
            print_netroute_addr(route);
            printf(" (%s)\n", route->mac ? "mac" : (route->ip4 ? "ipv4" : "ipv6"));
            route = route->next;
        }
        peer = peer->next;
    }
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
    netroute_t route;
    peer_t *target = NULL;

    if (!buf) {
        fprintf(stderr, "Could not allocate memory for *databuf\n");
        exit(EXIT_FAILURE);
    }

    route.next = NULL;
    route.mac = false;
    route.ip4 = false;

    while ((n = tuntap_read(&buf[FRAME_HEADER_SIZE], sizeof(uint8_t) * FRAME_MAXSIZE)) > 0) {
        packet_srcaddr(&buf[FRAME_HEADER_SIZE], &route);
        if (!is_local_route(&route)) {
            printf("(local) ");
            add_netroute(&route, &local_routes);
        }
        packet_destaddr(&buf[FRAME_HEADER_SIZE], &route);
        encode_frame(buf, n, HEADER_DATA);
        if ((target = get_peer_route(&route))) {
            send_data_to_peer(buf, FRAME_HEADER_SIZE + n, target);
        } else {
            broadcast_data_to_peers(buf, FRAME_HEADER_SIZE + n, NULL);
        }
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

// return true if this route is the local tuntap device
bool is_local_route(netroute_t *route)
{
    netroute_t *routes = local_routes;

    while (routes) {
        if (compare_netroutes(route, routes))
            return true;
        routes = routes->next;
    }
    return false;
}

// return peer_t to which the route belongs, or NULL if we do not know this route
peer_t *get_peer_route(netroute_t *route)
{
    peer_t *peer = peers;

    while (peer) {
        if (peer->alive) {
            netroute_t *routes = peer->routes;

            while (routes) {
                if (compare_netroutes(route, routes))
                    return peer;
                routes = routes->next;
            }
        }

        peer = peer->next;
    }
    return NULL;
}