#include <stdbool.h>
#include <stdint.h>

#ifndef __TCP_TUNTAP_STRUCTS
#define __TCP_TUNTAP_STRUCTS

struct args {
    bool verbose;    // display more information for debugging
    bool tap_mode;   // is the interface TAP or TUN
    bool server;     // do we run in server or client mode
    char *address;   // address to connect or listen to
    uint16_t port;   // port to connect to or listen on
};

#endif