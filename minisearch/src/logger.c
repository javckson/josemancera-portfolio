
#include "logger.h"
#include <stdarg.h>
#include <string.h>

static LogLevel CURRENT = LOG_INFO;

void log_set_level(LogLevel lvl){ CURRENT = lvl; }
void log_set_level_str(const char *lvl){
    if (!lvl) return;
    if (!strcmp(lvl, "DEBUG")) CURRENT = LOG_DEBUG;
    else if (!strcmp(lvl, "INFO")) CURRENT = LOG_INFO;
    else if (!strcmp(lvl, "WARN")) CURRENT = LOG_WARN;
    else if (!strcmp(lvl, "ERROR")) CURRENT = LOG_ERROR;
}

void log_log(LogLevel lvl, const char *file, int line, const char *fmt, ...) {
    if (lvl < CURRENT) return;
    const char *name = lvl==LOG_DEBUG?"DEBUG":lvl==LOG_INFO?"INFO":lvl==LOG_WARN?"WARN":"ERROR";
    fprintf(stderr, "[%s] %s:%d: ", name, file, line);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}
