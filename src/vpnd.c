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

#include "args.h"
#include "tcp.h"
#include "interface.h"
#include "vpnd.h"
#include "peer.h"
#include "signals.h"
#include "scripts.h"
#include "logger.h"
#include "protocol.h"
#include "rsa.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

int vpnd(struct args *args)
{
    pthread_t broadcast_thread;


    if (!load_daemon_privkey() || !load_daemon_pubkey()) {
        free_daemon_keys();
        return EXIT_FAILURE;
    }

    load_trusted_keys();
    atexit(free_daemon_keys);
    atexit(clear_trusted_keys);

    if (tuntap_open(args->dev, args->tap_mode) == -1)
        return EXIT_FAILURE;

    logger(LOG_INFO, "Opened device: %s", tuntap_devname());

    execute_script(D_SCRIPT_DEV_UP);
    atexit(destroy_peers);

    broadcast_thread = broadcast_tuntap_device();
    if (args->server) {
        tcp_server(!args->no_ipv4, !args->no_ipv6, 10);
    } else {
        tcp_client(args->address, args->port);
    }

    tuntap_close();
    destroy_peers();

    logger(LOG_WARN, "Waiting for broadcasting thread to end...");
    pthread_cancel(broadcast_thread);
    pthread_join(broadcast_thread, NULL);
    return EXIT_SUCCESS;
}

int main(int ac, char **av)
{
    struct args args;

    parse_cmdline_arguments(ac, av, &args);

    if (args.detach) {
        pid_t pid = fork();
        if (pid == -1) {
            logger(LOG_CRIT, "fork failed: %s\n", strerror(errno));
            return EXIT_FAILURE;
        } else if (pid > 0) {
            return EXIT_SUCCESS;
        }
    }

    logger(LOG_INFO, "Running in %s mode", args.tap_mode ? "TAP" : "TUN");
    logger(LOG_DEBUG, "Address: %s", args.address);
    logger(LOG_DEBUG, "Port: %u", args.port);
    override_signals();

    return vpnd(&args);
}