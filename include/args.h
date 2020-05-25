#include "structs.h"

#ifndef DEFAULT_PORT
#define DEFAULT_PORT 9678
#endif

#ifndef __TCP_TUNTAP_ARGS
#define __TCP_TUNTAP_ARGS

void print_usage(const char *bin_name);
void print_help(const char *bin_name);
void initialize_default_args(struct args *args);
void parse_cmdline_arguments(int ac, char **av, struct args *args);

#endif