#include "scripts.h"
#include "interface.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

// execute a script
int execute_script(const char *script_path)
{
    setenv("INTERFACE", tuntap_devname(), 1);

    int ret = 0;

    if ((ret = system(script_path)) < 0) {
        logger(LOG_ERROR, "Script error: %s: %s", script_path, strerror(errno));
    }
    return ret;
}

// execute dev-up script
int execute_dev_up(void)
{
    return execute_script(D_SCRIPT_DEV_UP);
}