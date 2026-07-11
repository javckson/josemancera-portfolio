
#include "config.h"
#include "logger.h"
#include "index.h"
#include "filewalker.h"
#include "search.h"
#include "strutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void ensure_sample_docs(const char *root){
    struct stat st;
    if (stat(root, &st) == 0) return; // exists
#ifdef _WIN32
    mkdir(root);
#else
    mkdir(root, 0755);
#endif
    char path[512];
    snprintf(path, sizeof(path), "%s/doc1.txt", root);
    FILE *a = fopen(path, "w");
    fprintf(a, "Pointers and memory management are crucial in C. The stack grows and the heap allocates.\n");
    fclose(a);
    snprintf(path, sizeof(path), "%s/doc2.txt", root);
    FILE *b = fopen(path, "w");
    fprintf(b, "Assembly and reverse engineering often require hex editors and careful analysis.\n");
    fclose(b);
    snprintf(path, sizeof(path), "%s/doc3.txt", root);
    FILE *c = fopen(path, "w");
    fprintf(c, "Stacks, queues, and hash maps are data structures. Pointers point to memory.\n");
    fclose(c);
}

int main(int argc, char **argv){
    Config cfg;
    if(!parse_args(argc, argv, &cfg)) return 2;
    log_set_level_str(cfg.log_level);
    ensure_sample_docs(cfg.root_dir);

    InvertedIndex *idx = index_create();
    if(!walk_and_index(cfg.root_dir, idx)){
        LOGE("Indexing failed");
        index_free(idx);
        return 1;
    }
    LOGI("Indexed %zu documents; %zu unique terms", idx->doc_count, hm_size(idx->map));
    if(cfg.query){
        SearchHit *hits=NULL;
        size_t n = search_query(idx, cfg.query, &hits);
        for(size_t i=0;i<n;i++){
            const Document *d = index_get_doc(idx, hits[i].doc_id);
            printf("%3zu. score=%4.1f  %s\n", i+1, hits[i].score, d?d->path:"<unknown>");
        }
        free(hits);
    } else {
        printf("No query provided. Try: --query \"pointer OR stack\"\n");
    }
    index_free(idx);
    return 0;
}
