#include "protocol.h"
#include <stdio.h>
#include <string.h>

// decode and return payload size from header
uint32_t get_payload_size(uint8_t *buf)
{
    uint32_t data_len = 0;

    for (uint8_t i = 0; i < sizeof(uint32_t); ++i) {
        data_len |= buf[i] << ((3 - i) * 8);
    }
    return data_len;
}

// encode payload size into header
void set_payload_size(uint8_t *buf, uint32_t size)
{
    for (uint8_t i = 0; i < 4; ++i) {
        buf[i + 1] |= ((size >> ((3 - i) * 8)) & 0xff);
    }
}

// get header type
uint8_t header_type(uint8_t *buf)
{
    return *buf;
}

// send a data frame on socket s
void encode_frame(uint8_t *buf, size_t data_len, uint8_t type)
{
    *buf = type;
    set_payload_size(buf, data_len);
}