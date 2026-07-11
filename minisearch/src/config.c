
#include "config.h"
#include <string.h>
#include <stdio.h>

bool parse_args(int argc, char **argv, Config *out) {
    out->root_dir = NULL;
    out->query = NULL;
    out->log_level = "INFO";
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--root") && i+1 < argc) {
            out->root_dir = argv[++i];
        } else if (!strcmp(argv[i], "--query") && i+1 < argc) {
            out->query = argv[++i];
        } else if (!strcmp(argv[i], "--log") && i+1 < argc) {
            out->log_level = argv[++i];
        } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            printf("Usage: minisearch --root <dir> [--query \"...\"] [--log DEBUG|INFO|WARN|ERROR]\n");
            return false;
        } else {
            fprintf(stderr, "Unknown arg: %s\n", argv[i]);
            return false;
        }
    }
    if (!out->root_dir) {
        fprintf(stderr, "--root is required\n");
        return false;
    }
    return true;
}
