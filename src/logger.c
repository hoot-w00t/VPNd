#include "logger.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

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

        print_logging_level_str(stream, level);
        fprintf(stream, ": ");
        vfprintf(stream, format, ap);
        fprintf(stream, "\n");
        va_end(ap);
    }
}