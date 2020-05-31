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

#include "scripts.h"
#include "interface.h"
#include "logger.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

// execute a script
int execute_script(const char *script_name)
{
    char *path = join_paths(daemon_config_dir(), script_name);

    if (!path) {
        logger(LOG_CRIT, "Could not allocate memory for script path");
        return -1;
    }

    setenv("INTERFACE", tuntap_devname(), 1);

    int ret = 0;
    logger(LOG_DEBUG, "Executing script: %s", path);
    if ((ret = system(path)) < 0) {
        logger(LOG_ERROR, "Script error: %s: %s", path, strerror(errno));
    }
    return ret;
}