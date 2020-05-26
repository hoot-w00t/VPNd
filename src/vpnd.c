#include "attributes.h"
#include "args.h"
#include "tcp.h"
#include "interface.h"
#include "vpnd.h"
#include "peer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

in_addr_t get_ip_network(in_addr_t ip, uint8_t cidr)
{
    in_addr_t mask = 0;
    uint8_t cidr_limit = sizeof(in_addr_t) * 8;

    for (uint8_t i = 0; i < cidr && i < cidr_limit; ++i) {
        mask |= 1 << i;
    }
    return ip & mask;
}

int execute_command(const char *cmd)
{
    int ret = 0;

    printf("Executing '%s'\n", cmd);
    if ((ret = system(cmd))) {
        fprintf(stderr, "Execution failed, returned %i\n", ret);
    }
    return ret;
}

void set_device_ip(const char *ip_addr, uint8_t cidr, const char *dev)
{
    struct in_addr vpn_ip, vpn_network;

    vpn_ip.s_addr = inet_addr(ip_addr);
    vpn_network.s_addr = get_ip_network(vpn_ip.s_addr, cidr);

    char link_up[16 + IFNAMSIZ];
    char addr_add[36 + IFNAMSIZ];

    snprintf(link_up, sizeof(link_up), "ip link set %s up", dev);
    snprintf(addr_add, sizeof(addr_add), "ip addr add %s/%u dev %s",
             inet_ntoa(vpn_ip),
             cidr,
             dev);

    if (execute_command(link_up))
        return;
    if (execute_command(addr_add))
        return;

    printf("Set IP address on %s to %s\n", dev, inet_ntoa(vpn_ip));
    printf("Set network on %s to %s/%u\n", dev, inet_ntoa(vpn_network), cidr);
}

int main(int ac, char **av)
{
    struct args args;

    parse_cmdline_arguments(ac, av, &args);
    if (args.verbose) {
        printf("Running in %s mode\n", args.tap_mode ? "TAP" : "TUN");
        printf("Address: %s\n", args.address);
        printf("Port: %u\n", args.port);
        printf("Device name: %s\n", args.dev);
    }

    if (tuntap_open(args.dev, args.tap_mode) == -1)
        return EXIT_FAILURE;
    if (args.verbose)
        printf("Opened device: %s\n", args.dev);

    if (args.vpn_ip[0])
        set_device_ip(args.vpn_ip, args.vpn_cidr, args.dev);

    broadcast_dev_to_peers(false);
    if (args.server) {
        tcp_server(args.address, args.port);
    } else {
        tcp_client(args.address, args.port);
    }
    return EXIT_SUCCESS;
}