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
#include "protocol.h"
#include "logger.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <arpa/inet.h>

// write netroute address to *dest
void get_netroute_addr(netroute_t *route, char *dest, uint16_t maxlen)
{
    if (route->mac) {
        snprintf(dest, maxlen, "%02x:%02x:%02x:%02x:%02x:%02x",
                                route->addr[0],
                                route->addr[1],
                                route->addr[2],
                                route->addr[3],
                                route->addr[4],
                                route->addr[5]);
    } else if (route->ip4) {
        if (is_little_endian()) {
            byte_t ordered_ip[4] = {
                route->addr[3],
                route->addr[2],
                route->addr[1],
                route->addr[0]
            };

            inet_ntop(AF_INET, ordered_ip, dest, maxlen);
        } else {
            inet_ntop(AF_INET, route->addr, dest, maxlen);
        }
    } else {
        inet_ntop(AF_INET6, route->addr, dest, maxlen);
    }
}

// if the two netroutes contain the same address and type, return true
bool compare_netroutes(const netroute_t *r1, const netroute_t *r2)
{
    byte_t limit = 6;

    if (r1->mac != r2->mac || r1->ip4 != r2->ip4)
        return false;

    if (!r1->mac)
        limit = r1->ip4 ? 4 : sizeof(r1->addr);

    if (memcmp(r1->addr, r2->addr, limit))
        return false;

    return true;
}

// duplicate route
netroute_t *duplicate_netroute(const netroute_t *route)
{
    netroute_t *dupr = malloc(sizeof(netroute_t));

    if (!dupr)
        return NULL;

    dupr->mac = route->mac;
    dupr->ip4 = route->ip4;
    memcpy(dupr->addr, route->addr, sizeof(route->addr));
    dupr->next = route->next;

    return dupr;
}

// append *route to an array
void add_netroute(const netroute_t *route, netroute_t **array)
{
    netroute_t *dupr = duplicate_netroute(route);

    if (!dupr) {
        logger(LOG_CRIT, "Could not allocate memory for netroute");
        return;
    }

    dupr->next = NULL;
    if (*array == NULL) {
        *array = dupr;
    } else {
        netroute_t *last = *array;

        while (last->next)
            last = last->next;
        last->next = dupr;
    }
}

// free netroutes
void destroy_netroutes(netroute_t *routes)
{
    netroute_t *current = routes;
    netroute_t *next = NULL;

    while (current) {
        next = current->next;
        free(current);
        current = next;
    }
}

// search for *route in *array
// if found return the matching route
// otherwise return NULL
netroute_t *netroute_in_array(const netroute_t *route, netroute_t *array)
{
    for (netroute_t *i = array; i != NULL; i = i->next) {
        if (compare_netroutes(route, i))
            return i;
    }

    return NULL;
}