#include "netroute.h"
#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <arpa/inet.h>

// if the two netroutes contain the same address and type, return true
bool compare_netroutes(const netroute_t *r1, const netroute_t *r2)
{
    uint8_t limit = 6;

    if (r1->mac != r2->mac || r1->ip4 != r2->ip4)
        return false;

    if (!r1->mac) {
        if (r1->ip4) {
            limit = 4;
        } else {
            limit = sizeof(r1->addr);
        }
    }
    for (uint8_t i = 0; i < limit; ++i) {
        if (r1->addr[i] != r2->addr[i])
            return false;
    }
    return true;
}

// duplicate route
netroute_t *duplicate_netroute(const netroute_t *route)
{
    netroute_t *dupr = malloc(sizeof(netroute_t));

    if (!dupr)
        return NULL;

    dupr->mac = route->mac;
    dupr->ip4 = route->ip4;
    for (uint8_t i = 0; i < sizeof(route->addr); ++i)
        dupr->addr[i] = route->addr[i];
    dupr->next = route->next;

    return dupr;
}

// append *route to an array
void add_netroute(netroute_t *route, netroute_t **array)
{
    netroute_t *dupr = duplicate_netroute(route);

    if (!dupr) {
        fprintf(stderr, "Could not allocate memory for netroute\n");
        return;
    }

    printf("adding route ");
    print_netroute_addr(route);
    printf("\n");
    dupr->next = NULL;
    if (*array == NULL) {
        *array = dupr;
    } else {
        netroute_t *last = *array;

        while (last->next)
            last = last->next;
        last->next = dupr;
    }
}

// free netroutes
void destroy_netroutes(netroute_t *routes)
{
    netroute_t *current = routes;
    netroute_t *next = NULL;

    while (current) {
        next = current->next;
        free(current);
        current = next;
    }
}

// print netroute address to stdout
void print_netroute_addr(netroute_t *route)
{
    if (route->mac) {
        printf("%02x:%02x:%02x:%02x:%02x:%02x",
               route->addr[0],
               route->addr[1],
               route->addr[2],
               route->addr[3],
               route->addr[4],
               route->addr[5]);
    } else if (route->ip4) {
        char ip_addr[INET_ADDRSTRLEN];

        if (is_little_endian()) {
            uint8_t ordered_ip[4] = {
                route->addr[3],
                route->addr[2],
                route->addr[1],
                route->addr[0]
            };

            inet_ntop(AF_INET, ordered_ip, ip_addr, sizeof(ip_addr));
        } else {
            inet_ntop(AF_INET, route->addr, ip_addr, sizeof(ip_addr));
        }

        printf("%s", ip_addr);
    } else {
        char ip_addr[INET6_ADDRSTRLEN];

        inet_ntop(AF_INET6, route->addr, ip_addr, sizeof(ip_addr));
        printf("%s", ip_addr);
    }
}