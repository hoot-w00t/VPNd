/*
    VPNd - a VPN daemon
    Copyright (C) 2020  akrocynova

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "attributes.h"
#include "args.h"
#include "tcp.h"
#include "interface.h"
#include "vpnd.h"
#include "peer.h"
#include "signals.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int vpnd(struct args *args)
{
    pthread_t broadcast_thread;

    if (tuntap_open(args->dev, args->tap_mode) == -1)
        return EXIT_FAILURE;

    if (args->verbose)
        printf("Opened device: %s\n", args->dev);

    if (*(args->vpn_ip))
        set_device_ip(args->vpn_ip, args->vpn_cidr, args->dev);

    atexit(destroy_peers);

    broadcast_thread = broadcast_tuntap_device();
    if (args->server) {
        tcp_server(args->address, args->port, 10);
    } else {
        tcp_client(args->address, args->port);
    }
    destroy_peers();
    tuntap_close();
    printf("Waiting for broadcasting thread to end...\n");
    pthread_cancel(broadcast_thread);
    pthread_join(broadcast_thread, NULL);
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
    override_signals();

    return vpnd(&args);
}