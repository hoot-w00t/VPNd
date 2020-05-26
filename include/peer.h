#include <stdbool.h>
#include <structs.h>
#include <netinet/in.h>

#ifndef _VPND_PEER
#define _VPND_PEER

void destroy_peers(void);
void peer_connection(struct sockaddr_in *sin, int s, bool is_client, bool block);
void broadcast_data_to_peers(uint8_t *data, size_t n, peer_t *exclude);
void broadcast_tuntap_device(bool block);

#endif