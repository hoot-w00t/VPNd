#include "attributes.h"
#include "args.h"
#include <stdio.h>

int main(int ac, char **av)
{
    struct args args;

    parse_cmdline_arguments(ac,av, &args);

    if (args.verbose) {
        printf("Running in %s mode\n", args.tap_mode ? "TAP" : "TUN");
        printf("Address: %s\n", args.address);
        printf("Port: %u\n", args.port);
    }
    return 0;
}