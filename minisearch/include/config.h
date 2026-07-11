
#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef struct {
    const char *root_dir;   // directory to index
    const char *query;      // query string (optional)
    const char *log_level;  // "DEBUG","INFO","WARN","ERROR"
} Config;

bool parse_args(int argc, char **argv, Config *out);
#endif
