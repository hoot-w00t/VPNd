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
#include <stdint.h>

#ifndef _VPND_TCP
#define _VPND_TCP

int tcp4_bind(const char *bind_address, const uint16_t bind_port,
    const int backlog);
int tcp6_bind(const char *bind_address, const uint16_t bind_port,
    const int backlog);

int tcp_accept_connection(const int s);
void tcp_server_close(void);
int tcp_server(const bool ip4, const bool ip6, const int backlog);
int tcp_client(const char *hostname, uint16_t port);

#endif