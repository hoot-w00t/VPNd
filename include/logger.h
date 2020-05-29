#ifndef _VPND_LOGGER
#define _VPND_LOGGER

#define LOG_DEBUG 5
#define LOG_INFO  4
#define LOG_WARN  3
#define LOG_ERROR 2
#define LOG_CRIT  1

#define LOG_DEFAULT LOG_WARN

int set_logging_level(int level);
int set_logging_level_str(const char *level);
void print_logging_levels_str(void);
void logger(int level, const char *format, ...);

#endif