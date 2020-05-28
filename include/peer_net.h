#include "structs.h"

#ifndef _VPND_PEER_NET
#define _VPND_PEER_NET

void sendall(int s, uint8_t *data, size_t n, int flags);
void send_data_to_peer(uint8_t *data, size_t n, peer_t *peer);
void broadcast_keepalive(void);
ssize_t receive_frame(uint8_t *buf, size_t n, peer_t *peer);
void *peer_receive(void *arg);

#endif