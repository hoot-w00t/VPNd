#include "structs.h"
#include "vpnd.h"

#ifndef DEFAULT_PORT
#define DEFAULT_PORT 9678
#endif

#ifndef _VPND_ARGS
#define _VPND_ARGS

void print_version(void);
void print_usage(const char *bin_name);
void print_help(const char *bin_name);
void initialize_default_args(struct args *args);
void parse_cmdline_arguments(int ac, char **av, struct args *args);

#endif