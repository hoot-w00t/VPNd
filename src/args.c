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
#include "vpnd.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <getopt.h>

// print license
void print_license(void)
{
    printf("VPNd  Copyright (C) 2020  akrocynova\n");
    printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
    printf("This is free software, and you are welcome to redistribute it\n");
    printf("under certain conditions.\n");
    printf("Read LICENSE for more information.\n");
}

// print program version
void print_version(void)
{
    printf("VPN daemon version %s\n", GITVER);
}

// print program usage to stdout
void print_usage(const char *bin_name)
{
    printf("Usage: %s [options] address\n", bin_name);
}

// print program help and usage to stdout
void print_help(const char *bin_name)
{
    print_license();
    printf("\n");
    print_usage(bin_name);
    printf("\nDescription:\n");
    printf("Optionnal arguments:\n");
    printf("    -h, --help           show this help message\n");
    printf("    -v, --version        display program version\n");
    printf("    --log-level {level}  set the log level (default=warn)\n\n");
    printf("    -s, --server         run in server mode\n");
    printf("    -i, --interface      interface name\n");
    printf("                         (default=vpnd)\n");
    printf("    -m, --mode {mode}    interface mode, can be either tun or tap\n");
    printf("                         (default: tun)\n");
    printf("    -p, --port {port}    port to connect to or listen on\n");
    printf("    --dev-up {script}    path to the script that will be executed\n");
    printf("                         after the TUN/TAP device is opened\n");
    printf("                         (default=./dev-up)\n");
    printf("    --detach             detach the process to run as a daemon\n");
    printf("\nPositionnal argument:\n");
    printf("    address              address to connect to or listen on\n");
}

// initialize arguments to their default values
void initialize_default_args(struct args *args)
{
    args->detach = false;
    args->server = false;
    args->tap_mode = false;
    args->address = NULL;
    args->port = DEFAULT_PORT;
    memset(args->dev, 0, sizeof(args->dev));
    strncpy(args->dev, "vpnd", IFNAMSIZ - 1);
}

// parse interface mode
void parse_interface_mode(const char *arg, struct args *args)
{
    if (!strcmp(arg, "tun")) {
        args->tap_mode = false;
    } else if (!strcmp(arg, "tap")) {
        args->tap_mode = true;
    } else {
        fprintf(stderr, "Invalid mode: %s\n", arg);
        exit(EXIT_FAILURE);
    }
}

// parse port
void parse_port(const char *arg, struct args *args)
{
    int port = atoi(arg);

    if (port <= 0 || port > UINT16_MAX) {
        fprintf(stderr, "Invalid port: %s\n", arg);
        exit(EXIT_FAILURE);
    }
    args->port = (uint16_t) port;
}

// parse log level
void parse_log_level(const char *arg)
{
    if (set_logging_level_str(arg) < 0) {
        fprintf(stderr, "Invalid logging level, possible levels are: ");
        print_logging_levels_str();
        printf("\n");
        exit(EXIT_FAILURE);
    }
}

// parse long opts
void parse_longopt(int opt_index, char **av, struct args *args)
{
    switch (opt_index) {
        case 0: // help
            print_help(av[0]);
            exit(EXIT_SUCCESS);

        case 1: // version
            print_version();
            exit(EXIT_SUCCESS);

        case 2: // log-level
            parse_log_level(optarg);
            break;

        case 3: // server
            args->server = true;
            break;

        case 4: // mode
            parse_interface_mode(optarg, args);
            break;

        case 5: // port
            parse_port(optarg, args);
            break;

        case 6: // interface
            strncpy(args->dev, optarg, IFNAMSIZ - 1);
            break;

        case 7: // detach
            args->detach = true;
            break;

        default:
            exit(EXIT_FAILURE);
    }
}

// parse short opts
void parse_shortopt(int opt, char **av, struct args *args)
{
    switch (opt) {
        case 'h': // display help and exit
            print_help(av[0]);
            exit(EXIT_SUCCESS);

        case 'v': // version
            print_version();
            exit(EXIT_SUCCESS);

        case 's': // server mode
            args->server = true;
            break;

        case 'm': // interface mode
            parse_interface_mode(optarg, args);
            break;

        case 'p': // port
            parse_port(optarg, args);
            break;

        case 'i': // interface name
            strncpy(args->dev, optarg, IFNAMSIZ - 1);
            break;

        default: // invalid option
            exit(EXIT_FAILURE);
    }
}

// parse command line arguments and place the parsed values into *args
// returns -1 if an error was encountered
// returns 0 if no error was encountered
void parse_cmdline_arguments(int ac, char **av, struct args *args)
{
    const struct option opts[] = {
        {"help",      no_argument,       0, 0},
        {"version",   no_argument,       0, 0},
        {"log-level", required_argument, 0, 0},
        {"server",    no_argument,       0, 0},
        {"mode",      required_argument, 0, 0},
        {"port",      required_argument, 0, 0},
        {"interface", required_argument, 0, 0},
        {"detach",    no_argument,       0, 0},
        {0, 0, 0, 0}
    };
    const char optstring[] = "hvsm:p:i:";
    int opt = 0;
    int opt_index = 0;

    initialize_default_args(args);
    while ((opt = getopt_long(ac, av, optstring, opts, &opt_index)) != -1) {
        if (opt == 0) {
            parse_longopt(opt_index, av, args);
        } else {
            parse_shortopt(opt, av, args);
        }
    }

    if (optind >= ac) {
        print_usage(av[0]);
        fprintf(stderr, "\nMissing positionnal argument: address\n");
        exit(EXIT_FAILURE);
    } else {
        args->address = av[optind];
    }
}