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
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifndef _VPND_INTERFACE
#define _VPND_INTERFACE

int tuntap_fildes(void);
bool tuntap_tap_mode(void);
const char *tuntap_devname(void);
void tuntap_close(void);
int tuntap_open(char *dev, bool tap_mode);
ssize_t tuntap_write(void *buf, size_t n);
ssize_t tuntap_read(void *buf, size_t nbytes);

#endif