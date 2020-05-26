#include "attributes.h"
#include "args.h"
#include "tcp.h"
#include "interface.h"
#include "vpnd.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int ac, char **av)
{
    char dev[16];
    char base[] = "tunnel";
    struct args args;

    parse_cmdline_arguments(ac,av, &args);

    if (args.verbose) {
        printf("Running in %s mode\n", args.tap_mode ? "TAP" : "TUN");
        printf("Address: %s\n", args.address);
        printf("Port: %u\n", args.port);
    }

    strcpy(dev, base);
    strcat(dev, args.server ? "0" : "1");
    if (tuntap_open(dev, args.tap_mode) == -1)
        return EXIT_FAILURE;
    if (args.verbose)
        printf("Device: %s\n", dev);
    if (args.server) {
        system("ifconfig tunnel0 192.168.2.1 netmask 255.255.255.0");
        printf("IP address: 192.168.2.1\n");
    } else {
        system("ifconfig tunnel1 192.168.2.2 netmask 255.255.255.0");
        printf("IP address: 192.168.2.2\n");
    }

    if (args.server) {
        tcp_server(args.address, args.port);
    } else {
        tcp_client(args.address, args.port);
    }
    return 0;
}