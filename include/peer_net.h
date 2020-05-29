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

#include "structs.h"

#ifndef _VPND_PEER_NET
#define _VPND_PEER_NET

void sendall(int s, uint8_t *data, size_t n, int flags);
void send_data_to_peer(uint8_t *data, size_t n, peer_t *peer);
void broadcast_keepalive(void);
void broadcast_close(void);
ssize_t receive_frame(uint8_t *buf, size_t n, peer_t *peer);
void *peer_receive(void *arg);

#endif