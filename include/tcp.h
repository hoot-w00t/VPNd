#include "structs.h"

#ifndef _VPND_TCP
#define _VPND_TCP

int tcp_bind(const char *bind_address, uint16_t bind_port, int backlog);
int tcp_accept_connection(void);
int tcp_server(const char *bind_address, uint16_t bind_port, int backlog);
void tcp_server_close(void);
int tcp_client(const char *address, uint16_t port);

#endif