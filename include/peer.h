/*
    VPNd - a VPN daemon
    Copyright (C) 2020  akrocynova

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
void dump_peers(void);
void peer_connection(struct sockaddr_in *sin, int s, bool is_client, bool block);
void broadcast_data_to_peers(byte_t *data, size_t n, peer_t *exclude);
pthread_t broadcast_tuntap_device(void);
bool is_local_route(netroute_t *route);
peer_t *get_peer_route(netroute_t *route);

#endif