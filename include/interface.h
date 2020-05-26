#include <stdbool.h>

#ifndef _VPND_INTERFACE
#define _VPND_INTERFACE

int tuntap_fildes(void);
void tuntap_close(void);
int tuntap_open(char *dev, bool tap_mode);
ssize_t tuntap_write(void *buf, size_t n);
ssize_t tuntap_read(void *buf, size_t nbytes);

#endif