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

#include "structs.h"
#include "vpnd.h"

#ifndef DEFAULT_PORT
#define DEFAULT_PORT 9678
#endif

#ifndef _VPND_ARGS
#define _VPND_ARGS

void print_license(void);
void print_version(void);
void print_usage(const char *bin_name);
void print_help(const char *bin_name);
void initialize_default_args(struct args *args);
void parse_cmdline_arguments(int ac, char **av, struct args *args);

#endif