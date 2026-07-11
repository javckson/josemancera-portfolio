
#ifndef LOGGER_H
#define LOGGER_H
#include <stdio.h>

typedef enum { LOG_DEBUG=0, LOG_INFO=1, LOG_WARN=2, LOG_ERROR=3 } LogLevel;
void log_set_level(LogLevel lvl);
void log_set_level_str(const char *lvl);
void log_log(LogLevel lvl, const char *file, int line, const char *fmt, ...);

#define LOGD(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOGI(...) log_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define LOGW(...) log_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define LOGE(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)

#endif
