#include "vpnd.h"
#include <stdint.h>
#include <stddef.h>

#ifndef _VPND_PROTOCOL
#define _VPND_PROTOCOL

#define FRAME_DATA_MAXSIZE 2048
#define FRAME_MAXSIZE (FRAME_DATA_MAXSIZE + sizeof(uint32_t))

/*

Frame format (big-endian byte order or MSB)

data_len == 4 bytes
data == data_len bytes

*/

struct vpnd_frame {
    uint8_t data[FRAME_DATA_MAXSIZE];      // actual data
    uint32_t data_len;  // length of data
};
typedef struct vpnd_frame vpnd_frame_t;

int decode_frame(uint8_t *buf, size_t bufsize, vpnd_frame_t *frame);
size_t encode_frame(uint8_t *buf, size_t bufsize, vpnd_frame_t *frame);

#endif