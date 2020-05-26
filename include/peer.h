#include <stdbool.h>
#include <structs.h>

#ifndef _VPND_PEER
#define _VPND_PEER

peer_t *create_peer(int s, bool is_client);
void destroy_peer(peer_t *peer);
void append_peer(peer_t *peer);
void destroy_peers(void);
void *peer_receive(void *arg);
void *_peer_connection(void *arg);
void peer_connection(int s, bool is_client, bool block);
void broadcast_data_to_peers(uint8_t *data, size_t n, peer_t *exclude);
void *_broadcast_dev_to_peers(void *arg);
void broadcast_dev_to_peers(bool block);

#endif