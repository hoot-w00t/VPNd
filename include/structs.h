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
#include <stdint.h>
#include <linux/if.h>
#include <pthread.h>

#ifndef _VPND_STRUCTS
#define _VPND_STRUCTS

struct args {
    bool detach;           // should the process fork() to detach
    bool tap_mode;         // is the interface TAP or TUN
    bool server;           // do we run in server or client mode
    char dev[IFNAMSIZ];    // device name
    char *address;         // address to connect or listen to
    uint16_t port;         // port to connect to or listen on
};

typedef struct peer peer_t;
typedef struct netroute netroute_t;
struct peer {
    pthread_mutex_t mutex;     // thread mutex
    int s;                     // socket descriptor
    bool is_client;            // is it a client or a server connection
    char *address;             // remote address
    uint16_t port;             // remote port
    bool alive;                // is this peer connected
    struct netroute *routes;   // peer routes
    struct peer *next;         // next peer in the linked list
};

struct netroute {
    bool mac;              // is addr a MAC address
    bool ip4;              // if !mac is addr an IPv4 or an IPv6 address
    uint8_t addr[16];      // route address bytes (in big endian)
    struct netroute *next; // next route in the linked list
};

#endif