#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifndef _VPND_INTERFACE
#define _VPND_INTERFACE

int tuntap_fildes(void);
bool tuntap_tap_mode(void);
void tuntap_close(void);
int tuntap_open(char *dev, bool tap_mode);
ssize_t tuntap_write(void *buf, size_t n);
ssize_t tuntap_read(void *buf, size_t nbytes);
void set_device_ip(const char *ip_addr, uint8_t cidr, const char *dev);

#endif