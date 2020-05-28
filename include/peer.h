#include <stdbool.h>
#include <structs.h>
#include <netinet/in.h>
#include <pthread.h>

#ifndef _VPND_PEER
#define _VPND_PEER

void initialize_peer(peer_t *peer);
void set_peer_info(peer_t *peer, struct sockaddr_in *sin, int s, bool is_client);
peer_t *create_peer(struct sockaddr_in *sin, int s, bool is_client);
void destroy_peer(peer_t *peer);
void destroy_peers(void);
peer_t *add_peer(struct sockaddr_in *sin, int s, bool is_client);
void peer_connection(struct sockaddr_in *sin, int s, bool is_client, bool block);
void broadcast_data_to_peers(uint8_t *data, size_t n, peer_t *exclude);
pthread_t broadcast_tuntap_device(void);

#endif