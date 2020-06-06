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

#ifndef _VPND_NETROUTE
#define _VPND_NETROUTE

void get_netroute_addr(netroute_t *route, char *dest, uint16_t maxlen);
bool compare_netroutes(const netroute_t *r1, const netroute_t *r2);
netroute_t *duplicate_netroute(const netroute_t *route);
void add_netroute(const netroute_t *route, netroute_t **array);
void destroy_netroutes(netroute_t *routes);
void print_netroute_addr(netroute_t *route);
netroute_t *netroute_in_array(const netroute_t *route, netroute_t *array);

#endif