#include "protocol.h"
#include <stdio.h>
#include <string.h>

uint32_t get_frame_data_len(uint8_t *buf)
{
    uint32_t data_len = 0;

    for (uint8_t i = 0; i < sizeof(uint32_t); ++i) {
        data_len |= buf[i] << ((sizeof(uint32_t) - 1 - i) * 8);
    }
    return data_len;
}

void set_frame_data_len(uint8_t *buf, vpnd_frame_t *frame)
{
    for (uint8_t i = 0; i < sizeof(uint32_t); ++i) {
        buf[i] = (frame->data_len >> ((sizeof(uint32_t) - 1 - i) * 8)) & 0xff;
    }
}

// decode raw frame stored in buf and put decoded data in *frame
int decode_frame(uint8_t *buf, size_t bufsize, vpnd_frame_t *frame)
{
    if (bufsize <= sizeof(uint32_t)) {
        fprintf(stderr, "Invalid frame header: size too short\n");
        return -1;
    }

    frame->data_len = get_frame_data_len(buf);
    memcpy(frame->data, &buf[sizeof(uint32_t)], frame->data_len);
    return 0;
}

// encode data in *frame into *buf, if there is more data than there is
// space in *buf it will be lost
size_t encode_frame(uint8_t *buf, size_t bufsize, vpnd_frame_t *frame)
{
    set_frame_data_len(buf, frame);
    for (size_t i = sizeof(uint32_t); i < bufsize; ++i) {
        buf[i] = frame->data[i - sizeof(uint32_t)];
    }
    return sizeof(uint32_t) + frame->data_len;
}
