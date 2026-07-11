
#include "strutil.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

char* str_dup(const char *s){
    if(!s) return NULL;
    size_t n = strlen(s);
    char *p = (char*)malloc(n+1);
    if(!p) return NULL;
    memcpy(p, s, n+1);
    return p;
}

char* str_to_lower_dup(const char *s){
    if(!s) return NULL;
    size_t n = strlen(s);
    char *p = (char*)malloc(n+1);
    if(!p) return NULL;
    for(size_t i=0;i<n;i++) p[i] = (char)tolower((unsigned char)s[i]);
    p[n] = '\0';
    return p;
}

bool str_ends_with(const char *s, const char *suffix){
    size_t ns = strlen(s), nx = strlen(suffix);
    if(nx > ns) return false;
    return memcmp(s + ns - nx, suffix, nx) == 0;
}

char* read_entire_file(const char *path, size_t *out_len){
    FILE *f = fopen(path, "rb");
    if(!f) return NULL;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if(n < 0){ fclose(f); return NULL; }
    char *buf = (char*)malloc((size_t)n + 1);
    if(!buf){ fclose(f); return NULL; }
    size_t r = fread(buf, 1, (size_t)n, f);
    fclose(f);
    buf[r] = '\0';
    if(out_len) *out_len = r;
    return buf;
}
