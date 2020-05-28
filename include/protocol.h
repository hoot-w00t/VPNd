#include "vpnd.h"
#include <stdint.h>
#include <stddef.h>

#ifndef _VPND_PROTOCOL
#define _VPND_PROTOCOL

#define FRAME_HEADER_SIZE     (1 + sizeof(uint32_t))
#define FRAME_PAYLOAD_MAXSIZE 2048
#define FRAME_MAXSIZE (FRAME_HEADER_SIZE + FRAME_PAYLOAD_MAXSIZE)

#define HEADER_KEEPALIVE 0b1
#define HEADER_DATA      0b10

/*

Frame format

header type == 1 byte
data_len == 4 bytes (big-endian)
payload == data_len bytes

*/

int is_little_endian(void);
uint32_t read_uint32(uint8_t *buf);
void write_uint32(uint8_t *buf, uint32_t size);
void encode_frame(uint8_t *buf, size_t data_len, uint8_t type);

#endif