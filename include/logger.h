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