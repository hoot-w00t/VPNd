#include <stdbool.h>
#include <stdint.h>
#include <linux/if.h>

#ifndef _VPND_STRUCTS
#define _VPND_STRUCTS

struct args {
    bool verbose;       // display more information for debugging
    bool tap_mode;      // is the interface TAP or TUN
    bool server;        // do we run in server or client mode
    char dev[IFNAMSIZ]; // device name
    char *address;      // address to connect or listen to
    uint16_t port;      // port to connect to or listen on
};

typedef struct peer peer_t;
struct peer {
    int s;                     // socket descriptor
    bool is_client;            // is it a client or a server connection
    bool alive;                // is this peer connected
    struct peer *next;         // next peer in the linked list
};

#endif