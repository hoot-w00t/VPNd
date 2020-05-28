#include "structs.h"

#ifndef _VPND_NETROUTE
#define _VPND_NETROUTE

bool compare_netroutes(const netroute_t *r1, const netroute_t *r2);
netroute_t *duplicate_netroute(const netroute_t *route);
void add_netroute(netroute_t *route, netroute_t **array);
void destroy_netroutes(netroute_t *routes);
void print_netroute_addr(netroute_t *route);

#endif