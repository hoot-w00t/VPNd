#include "netroute.h"

#ifndef _VPND_PACKET_HEADER
#define _VPND_PACKET_HEADER

void packet_srcaddr(uint8_t *packet, netroute_t *dest);
void packet_destaddr(uint8_t *packet, netroute_t *dest);

#endif