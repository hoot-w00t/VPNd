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

#include "protocol.h"
#include <stdio.h>
#include <string.h>

// are we running on a little or big endian machine
int is_little_endian(void)
{
    int n = 1;

    return (*(char *) &n == 1);
}

// read uint32 from *buf in big endian
uint32_t read_uint32(uint8_t *buf)
{
    uint32_t data_len = 0;

    if (is_little_endian()) {
        data_len |= (buf[3] << 24);
        data_len |= (buf[2] << 16);
        data_len |= (buf[1] << 8);
        data_len |= buf[0];
    } else {
        data_len |= (buf[0] << 24);
        data_len |= (buf[1] << 16);
        data_len |= (buf[2] << 8);
        data_len |= buf[3];
    }
    return data_len;
}

// write uint32 at *buf in big endian
void write_uint32(uint8_t *buf, uint32_t size)
{
    if (is_little_endian()) {
        buf[3] = (uint8_t) ((size >> 24) & 0xff);
        buf[2] = (uint8_t) ((size >> 16) & 0xff);
        buf[1] = (uint8_t) ((size >> 8) & 0xff);
        buf[0] = (uint8_t) (size & 0xff);
    } else {
        buf[0] = (uint8_t) ((size >> 24) & 0xff);
        buf[1] = (uint8_t) ((size >> 16) & 0xff);
        buf[2] = (uint8_t) ((size >> 8) & 0xff);
        buf[3] = (uint8_t) (size & 0xff);
    }
}

// send a data frame on socket s
void encode_frame(uint8_t *buf, size_t data_len, uint8_t type)
{
    *buf = type;
    write_uint32(&buf[1], data_len);
}