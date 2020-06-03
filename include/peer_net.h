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

void send_data_to_peer(uint8_t frame_hdr_type, byte_t *data, uint32_t data_len,
    bool encrypt, peer_t *peer);
void broadcast_data_to_peers(uint8_t frame_hdr_type, byte_t *data, uint32_t data_len,
    bool encrypt, peer_t *exclude);
int receive_frame(peer_t *peer, byte_t *buf, uint8_t *header_type, uint32_t *data_len);
void process_netpacket(peer_t *peer, byte_t *data, uint32_t data_len);
int process_frame(peer_t *peer, byte_t *buf, uint8_t header_type, uint32_t data_len);
bool authenticate_peer(peer_t *peer, byte_t *buf);
void *peer_receive(void *arg);

#endif