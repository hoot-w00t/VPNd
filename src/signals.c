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

#include "tcp.h"
#include "peer.h"
#include "peer_net.h"
#include "protocol.h"
#include "interface.h"
#include "logger.h"
#include <stdio.h>
#include <signal.h>

void interrupt_program_handler(int sig)
{
    logger(LOG_DEBUG, "interrupt_program_handler (signal: %i)", sig);
    tcp_server_close();
    broadcast_data_to_peers(FRAME_HDR_CLOSE, NULL, 0, false, NULL);
    tuntap_close();
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