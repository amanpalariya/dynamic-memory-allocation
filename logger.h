#ifndef CS303_LOGGER_H
#define CS303_LOGGER_H

#define LOG_LEVEL_DEBUG (0)
#define LOG_LEVEL_INFO (1)
#define LOG_LEVEL_WARNING (2)
#define LOG_LEVEL_ERROR (3)
#define LOG_LEVEL_STAT (4)

#include <stdarg.h>
#include <stdio.h>

extern int __log_level__;
extern int __log_level_backup__;
extern FILE* __log_stream__;

void set_log_stream(FILE* stream);

void set_log_level(int new_log_level);

void mute_logs();

void unmute_logs();

void console_log(int log_level, const char* fmt, va_list args);

void log_debug(const char* fmt, ...);

void log_info(const char* fmt, ...);

void log_warning(const char* fmt, ...);

void log_error(const char* fmt, ...);

void log_stat(const char* fmt, ...);

#endif