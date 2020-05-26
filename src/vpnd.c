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

int vpnd(struct args *args)
{
    if (tuntap_open(args->dev, args->tap_mode) == -1)
        return EXIT_FAILURE;

    if (args->verbose)
        printf("Opened device: %s\n", args->dev);

    if (*(args->vpn_ip))
        set_device_ip(args->vpn_ip, args->vpn_cidr, args->dev);

    broadcast_tuntap_device(false);
    if (args->server) {
        tcp_server(args->address, args->port);
    } else {
        tcp_client(args->address, args->port);
    }
    return EXIT_SUCCESS;
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

    return vpnd(&args);
}