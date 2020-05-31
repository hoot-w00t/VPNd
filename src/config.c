#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

static char *config_dir = NULL;

// join two paths, adding a / in between if needed
char *join_paths(const char *path1, const char *path2)
{
    size_t path1_len = strlen(path1);
    char *result = malloc(sizeof(char) * (path1_len + strlen(path2) + 2));

    if (!result)
        return NULL;

    strcpy(result, path1);
    if (path1[path1_len - 1] != '/' && path2[0] != '/') {
        strcat(result, "/");
        strcat(result, path2);
    } else if (path1[path1_len - 1] == '/' && *path2 == '/')  {
        strcat(result, path2 + 1);
    } else {
        strcat(result, path2);
    }

    return result;
}

// return the daemon configuration directory
const char *daemon_config_dir(void)
{
    return config_dir;
}

// free config_dir
void free_config_dir(void)
{
    if (config_dir)
        free(config_dir);
}

// set config_dir
const char *set_config_dir(const char *dir)
{
    free_config_dir();
    config_dir = strdup(dir);

    return config_dir;
}