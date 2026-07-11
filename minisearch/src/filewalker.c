
#include "filewalker.h"
#include "strutil.h"
#include "tokenizer.h"
#include "logger.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static int is_dir(const char *path){
    struct stat st;
    if(stat(path, &st) != 0) return 0;
    return S_ISDIR(st.st_mode);
}

static int is_regular_file(const char *path){
    struct stat st;
    if(stat(path, &st) != 0) return 0;
    return S_ISREG(st.st_mode);
}

static void index_file(const char *path, InvertedIndex *idx){
    size_t len=0;
    char *content = read_entire_file(path, &len);
    if(!content){
        LOGW("Could not read %s", path);
        return;
    }
    int doc_id = index_add_document(idx, path);
    // Tokenize in-place (mutable)
    char *buf = content;
    const size_t MAXTOK = 1<<14;
    char **tokens = (char**)malloc(MAXTOK*sizeof(char*));
    size_t n = tokenize(buf, tokens, MAXTOK);
    for(size_t i=0;i<n;i++){
        index_add_term(idx, tokens[i], doc_id);
    }
    free(tokens);
    free(content);
}

static bool walk_dir(const char *root, InvertedIndex *idx){
    DIR *d = opendir(root);
    if(!d){ LOGE("Failed to open dir %s", root); return false; }
    struct dirent *ent;
    while((ent = readdir(d))){
        if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) continue;
        char path[4096];
        snprintf(path, sizeof(path), "%s/%s", root, ent->d_name);
        if(is_dir(path)){
            walk_dir(path, idx);
        } else if(is_regular_file(path) && str_ends_with(path, ".txt")){
            LOGD("Indexing %s", path);
            index_file(path, idx);
        }
    }
    closedir(d);
    return true;
}

bool walk_and_index(const char *root, InvertedIndex *idx){
    return walk_dir(root, idx);
}
