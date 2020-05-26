#include "args.h"
#include "vpnd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <getopt.h>

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
    printf("Optionnal arguments:\n");
    printf("    -h, --help         show this help message\n");
    printf("    -v, --version      display program version\n");
    printf("    --verbose          display more information for debugging\n\n");
    printf("    -s, --server       run in server mode\n");
    printf("    -i, --interface    interface name\n");
    printf("                       (default=vpnd)\n");
    printf("    -m, --mode {mode}  interface mode, can be either tun or tap\n");
    printf("                       (default: tun)\n");
    printf("    -p, --port {port}  port to connect to or listen on\n");
    printf("\nPositionnal argument:\n");
    printf("    address            address to connect to or listen on\n");
}

// initialize arguments to their default values
void initialize_default_args(struct args *args)
{
    args->verbose = false;
    args->server = false;
    args->tap_mode = false;
    args->address = NULL;
    args->port = DEFAULT_PORT;
    memset(args->dev, 0, IFNAMSIZ);
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

        case 2: // verbose
            args->verbose = true;
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
        {"help", no_argument, 0, 0},
        {"version", no_argument, 0, 0},
        {"verbose", no_argument, 0, 0},
        {"server", no_argument, 0, 0},
        {"mode", required_argument, 0, 0},
        {"port", required_argument, 0, 0},
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