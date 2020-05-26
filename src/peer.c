#include "interface.h"
#include "peer.h"
#include "vpnd.h"
#include "connection.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>

// receive data from remote peer and write it to the tun/tap device
void *peer_receive(void *arg)
{
    connection_t *conn = (connection_t *) arg;
    uint8_t buf[VPND_BUFSIZE];
    ssize_t n = 0;

    while ((n = recv(conn->s, buf, VPND_BUFSIZE, 0)) > 0) {
        tuntap_write(buf, n);
    }
    return NULL;
}

// read from tun/tap device and send data to remote peer
void *peer_send(void *arg)
{
    connection_t *conn = (connection_t *) arg;
    uint8_t buf[VPND_BUFSIZE];
    ssize_t n = 0;

    while ((n = tuntap_read(buf, VPND_BUFSIZE)) > 0) {
        send(conn->s, buf, n, 0);
    }
    return NULL;
}

// handle a peer connection
int peer_connection(int s, bool is_client)
{
    connection_t *conn = create_connection(s, is_client);

    if (!conn)
        return -1;

    pthread_t thread_recv, thread_send;
    pthread_create(&thread_recv, NULL, peer_receive, conn);
    pthread_create(&thread_send, NULL, peer_send, conn);
    pthread_join(thread_recv, NULL);
    pthread_join(thread_send, NULL);
    destroy_connection(conn);
    return 0;
}