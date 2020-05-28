#include "peer_net.h"
#include "tcp.h"
#include "attributes.h"
#include <stdio.h>
#include <signal.h>

void interrupt_program_handler(UNUSED int sig)
{
    printf("Received SIGINT\n");
    tcp_server_close();
    broadcast_close();
}

void override_signals(void)
{
    signal(SIGINT, &interrupt_program_handler);
}