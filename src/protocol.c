#include "protocol.h"
#include <stdio.h>
#include <string.h>

int is_little_endian(void)
{
    int n = 1;

    return (*(char *) &n == 1);
}

// decode and return payload size from header
uint32_t get_payload_size(uint8_t *buf)
{
    uint32_t data_len = 0;

    if (is_little_endian()) {
        data_len |= (buf[4] << 24);
        data_len |= (buf[3] << 16);
        data_len |= (buf[2] << 8);
        data_len |= buf[1];
    } else {
        data_len |= (buf[1] << 24);
        data_len |= (buf[2] << 16);
        data_len |= (buf[3] << 8);
        data_len |= buf[4];
    }
    return data_len;
}

// encode payload size into header
void set_payload_size(uint8_t *buf, uint32_t size)
{
    if (is_little_endian()) {
        buf[4] = (uint8_t) ((size >> 24) & 0xff);
        buf[3] = (uint8_t) ((size >> 16) & 0xff);
        buf[2] = (uint8_t) ((size >> 8) & 0xff);
        buf[1] = (uint8_t) (size & 0xff);
    } else {
        buf[1] = (uint8_t) ((size >> 24) & 0xff);
        buf[2] = (uint8_t) ((size >> 16) & 0xff);
        buf[3] = (uint8_t) ((size >> 8) & 0xff);
        buf[4] = (uint8_t) (size & 0xff);
    }
}

// send a data frame on socket s
void encode_frame(uint8_t *buf, size_t data_len, uint8_t type)
{
    *buf = type;
    set_payload_size(buf, data_len);
}