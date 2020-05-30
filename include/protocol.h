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

#include "vpnd.h"
#include <openssl/rsa.h>
#include <stdint.h>
#include <stddef.h>

#ifndef _VPND_PROTOCOL
#define _VPND_PROTOCOL

#define FRAME_HEADER_SIZE     (1 + sizeof(uint32_t))
#define FRAME_PAYLOAD_MAXSIZE 2048
#define FRAME_MAXSIZE (FRAME_HEADER_SIZE + FRAME_PAYLOAD_MAXSIZE)

#define HEADER_KEEPALIVE 0b0001
#define HEADER_DATA      0b0010
#define HEADER_CLOSE     0b0100
#define HEADER_AUTH      0b1000

/*

Frame format

header type == 1 byte
data_len == 4 bytes (big-endian)
payload == data_len bytes

*/

RSA *get_daemon_privkey(void);
RSA *get_daemon_pubkey(void);
RSA *load_daemon_privkey(const char *filepath);
RSA *load_daemon_pubkey(const char *filepath);
void free_daemon_keys(void);
int is_little_endian(void);
uint32_t read_uint32(byte_t *buf);
void write_uint32(byte_t *buf, uint32_t size);
void encode_frame(byte_t *buf, size_t data_len, byte_t type);

#endif