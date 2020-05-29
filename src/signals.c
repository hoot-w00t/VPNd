#include "peer_net.h"
#include "tcp.h"
#include "peer.h"
#include "logger.h"
#include <stdio.h>
#include <signal.h>

void interrupt_program_handler(int sig)
{
    logger(LOG_DEBUG, "interrupt_program_handler (signal: %i)", sig);
    tcp_server_close();
    broadcast_close();
}

void usr1_handler(int sig)
{
    logger(LOG_DEBUG, "usr1_handler (signal: %i)", sig);
    dump_peers();
}

void override_signals(void)
{
    signal(SIGINT, &interrupt_program_handler);
    signal(SIGUSR1, &usr1_handler);
}