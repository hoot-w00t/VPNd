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

#include "netroute.h"

#ifndef _VPND_PACKET_HEADER
#define _VPND_PACKET_HEADER

void decode_tuntap_header(const byte_t *packet);
void parse_ip_packet(const byte_t *packet, const uint16_t proto,
    netroute_t *nroute, const bool source);
void parse_eth_packet(const byte_t *packet, netroute_t *nroute,
    const bool source);
void parse_packet_addr(const byte_t *packet, netroute_t *nroute,
    const bool source);

#endif