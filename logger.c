#include "logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int __log_level__ = 0;
int __log_level_backup__ = 0;
FILE* __log_stream__ = NULL;

void set_log_stream(FILE* stream) {
    __log_stream__ = stream;
}

void set_log_level(int new_log_level) {
    __log_level__ = new_log_level;
}

void mute_logs() {
    __log_level_backup__ = __log_level__;
    set_log_level(0x1000);
}

void unmute_logs() {
    __log_level__ = __log_level_backup__;
}

char* get_current_time_string() {
    time_t rawtime;
    time(&rawtime);
    struct tm* t = localtime(&rawtime);
    char* timestr = (char*)malloc(8 + 1);
    sprintf(timestr, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
    return timestr;
}

char* get_prefixed_format(int log_level, const char* fmt) {
    char* timestr = get_current_time_string();
    const int MAX_PREFIX_SIZE = 20;
    char prefix[MAX_PREFIX_SIZE];
    switch (log_level) {
        case LOG_LEVEL_INFO:
            sprintf(prefix, "INFO  %s:", timestr);
            break;
        case LOG_LEVEL_WARNING:
            sprintf(prefix, "WARN  %s:", timestr);
            break;
        case LOG_LEVEL_DEBUG:
            sprintf(prefix, "DEBUG %s:", timestr);
            break;
        case LOG_LEVEL_ERROR:
            sprintf(prefix, "ERROR %s:", timestr);
            break;
        case LOG_LEVEL_STAT:
            sprintf(prefix, "STAT  %s:", timestr);
            break;
        default:
            break;
    }
    free(timestr);
    char* new_fmt;
    new_fmt = (char*)malloc(strlen(fmt) + MAX_PREFIX_SIZE + 2);
    sprintf(new_fmt, "%s %s\n", prefix, fmt);
    return new_fmt;
}

void console_log(int log_level, const char* fmt, va_list args) {
    if (!__log_stream__) set_log_stream(stdout);
    if (log_level < __log_level__) return;
    char* new_fmt = get_prefixed_format(log_level, fmt);
    vfprintf(__log_stream__, new_fmt, args);
    free(new_fmt);
}

void log_debug(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    console_log(LOG_LEVEL_DEBUG, fmt, args);
    va_end(args);
}

void log_info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    console_log(LOG_LEVEL_INFO, fmt, args);
    va_end(args);
}

void log_warning(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    console_log(LOG_LEVEL_WARNING, fmt, args);
    va_end(args);
}

void log_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    console_log(LOG_LEVEL_ERROR, fmt, args);
    va_end(args);
}

void log_stat(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    console_log(LOG_LEVEL_STAT, fmt, args);
    va_end(args);
}