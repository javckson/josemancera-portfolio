
#ifndef STRUTIL_H
#define STRUTIL_H
#include <stddef.h>
#include <stdbool.h>

char* str_dup(const char *s);
char* str_to_lower_dup(const char *s);
bool str_ends_with(const char *s, const char *suffix);
char* read_entire_file(const char *path, size_t *out_len);
#endif
