#include "attributes.h"
#include "args.h"
#include "tcp.h"
#include "interface.h"
#include "vpnd.h"
#include "peer.h"
#include "signals.h"
#include "scripts.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int vpnd(struct args *args)
{
    pthread_t broadcast_thread;

    if (tuntap_open(args->dev, args->tap_mode) == -1)
        return EXIT_FAILURE;

    logger(LOG_INFO, "Opened device: %s", tuntap_devname());

    execute_dev_up();
    atexit(destroy_peers);

    broadcast_thread = broadcast_tuntap_device();
    if (args->server) {
        tcp_server(args->address, args->port, 10);
    } else {
        tcp_client(args->address, args->port);
    }

    destroy_peers();
    tuntap_close();
    logger(LOG_WARN, "Waiting for broadcasting thread to end...");
    pthread_cancel(broadcast_thread);
    pthread_join(broadcast_thread, NULL);
    return EXIT_SUCCESS;
}

int main(int ac, char **av)
{
    struct args args;

    parse_cmdline_arguments(ac, av, &args);

    logger(LOG_INFO, "Running in %s mode", args.tap_mode ? "TAP" : "TUN");
    logger(LOG_DEBUG, "Address: %s", args.address);
    logger(LOG_DEBUG, "Port: %u", args.port);
    override_signals();

    return vpnd(&args);
}