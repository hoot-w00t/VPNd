#include <stdbool.h>
#include <stdint.h>
#include <linux/if.h>
#include <pthread.h>

#ifndef _VPND_STRUCTS
#define _VPND_STRUCTS

struct args {
    bool verbose;                   // display more information for debugging
    bool tap_mode;                  // is the interface TAP or TUN
    bool server;                    // do we run in server or client mode
    char dev[IFNAMSIZ];             // device name
    char *address;                  // address to connect or listen to
    uint16_t port;                  // port to connect to or listen on
};

typedef struct peer peer_t;
typedef struct netroute netroute_t;
struct peer {
    pthread_mutex_t mutex;     // thread mutex
    int s;                     // socket descriptor
    bool is_client;            // is it a client or a server connection
    char *address;             // remote address
    uint16_t port;             // remote port
    bool alive;                // is this peer connected
    struct netroute *routes;   // peer routes
    struct peer *next;         // next peer in the linked list
};

struct netroute {
    bool mac;              // is addr a MAC address
    bool ip4;              // if !mac is addr an IPv4 or an IPv6 address
    uint8_t addr[16];      // route address bytes (in big endian)
    struct netroute *next; // next route in the linked list
};

#endif