#include "args.h"
#include "vpnd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

// print program version
void print_version(void)
{
    printf("VPN daemon version %s\n", GITVER);
}

// print program usage to stdout
void print_usage(const char *bin_name)
{
    size_t bin_len = strlen(bin_name);

    printf("Usage: %s [-h] [-v] [-s]\n", bin_name);
    printf("       ");
    for (size_t i = 0; i < bin_len; ++i)
        printf(" ");
    printf("[-m {mode}] [-p {port}] address\n");
}

// print program help and usage to stdout
void print_help(const char *bin_name)
{
    print_usage(bin_name);
    printf("\nDescription:\n\n");
    printf("    -h         show this help message\n");
    printf("    -v         display more information for debugging\n\n");
    printf("    -s         run in server mode\n");
    printf("    -m {mode}  interface mode, can be either tun or tap\n");
    printf("               (default: tun)\n");
    printf("    -p {port}  port to connect to or listen on\n");
    printf("    address    address to connect to or listen on\n");
}

// initialize arguments to their default values
void initialize_default_args(struct args *args)
{
    args->verbose = false;
    args->server = false;
    args->tap_mode = false;
    args->address = NULL;
    args->port = DEFAULT_PORT;
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

// parse command line arguments and place the parsed values into *args
// returns -1 if an error was encountered
// returns 0 if no error was encountered
void parse_cmdline_arguments(int ac, char **av, struct args *args)
{
    int opt = 0;

    initialize_default_args(args);
    while ((opt = getopt(ac, av, "hvsm:p:")) != -1) {
        switch (opt) {
            case 'h': // display help and exit
                print_help(av[0]);
                exit(EXIT_SUCCESS);

            case 'v': // verbose
                args->verbose = true;
                break;

            case 's': // server mode
                args->server = true;
                break;

            case 'm': // interface mode
                parse_interface_mode(optarg, args);
                break;

            case 'p': // port
                parse_port(optarg, args);
                break;

            default: // invalid option
                exit(EXIT_FAILURE);
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