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

Frame format (big-endian byte order or MSB)

header type == 1 byte
data_len == 4 bytes
payload == data_len bytes

*/

uint32_t get_payload_size(uint8_t *buf);
void set_payload_size(uint8_t *buf, uint32_t size);
uint8_t header_type(uint8_t *buf);
void encode_frame(uint8_t *buf, size_t data_len, uint8_t type);

#endif