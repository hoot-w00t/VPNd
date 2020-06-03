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

#include "logger.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pthread.h>

static pthread_mutex_t logger_mutex;
static int log_level = LOG_DEFAULT;

const char *log_levels_str[] = {
    "debug",
    "info",
    "warn",
    "error",
    "crit",
    NULL
};

const int log_levels[] = {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_CRIT,
    0
};

// destroy logger mutex
void free_logger(void)
{
    pthread_mutex_destroy(&logger_mutex);
}

// initialize logger mutex
void init_logger(void)
{
    pthread_mutex_init(&logger_mutex, NULL);
    atexit(free_logger);
}

// return -1 if level is invalid
// otherwise set to level and return 0
int set_logging_level(int level)
{
    if (level >= LOG_CRIT && level <= LOG_DEBUG) {
        log_level = level;
        return 0;
    }
    return -1;
}

// set logging level from a string
// return -1 if *level is invalid
int set_logging_level_str(const char *level)
{
    for (int i = 0; log_levels_str[i]; ++i) {
        if (!strcmp(level, log_levels_str[i])) {
            return set_logging_level(log_levels[i]);
        }
    }
    return -1;
}

// print logging levels to stdout
void print_logging_levels_str(void)
{
    for (int i = 0; log_levels_str[i]; ++i) {
        printf("%s", log_levels_str[i]);
        if (log_levels_str[i + 1]) {
            printf(", ");
        }
    }
}

// print logging level on *stream
void print_logging_level_str(FILE *stream, int level)
{
    for (int i = 0; log_levels[i]; ++i) {
        if (level == log_levels[i]) {
            fprintf(stream, "%s", log_levels_str[i]);
        }
    }
}

// log something
void logger(int level, const char *format, ...)
{
    if (level <= log_level) {
        va_list ap;
        va_start(ap, format);
        FILE *stream = level >= LOG_INFO ? stdout : stderr;

        pthread_mutex_lock(&logger_mutex);

        print_logging_level_str(stream, level);
        fprintf(stream, ": ");
        vfprintf(stream, format, ap);
        fprintf(stream, "\n");

        pthread_mutex_unlock(&logger_mutex);
        va_end(ap);
    }
}