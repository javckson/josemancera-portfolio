
#include "index.h"
#include "logger.h"
#include "strutil.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    Posting *data;
    size_t count;
    size_t cap;
} PostingVec;

typedef struct {
    char *term;
    PostingVec postings;
} TermEntry;

static void free_postings(void *val){
    PostingVec *v = (PostingVec*)val;
    free(v->data);
    free(v);
}

InvertedIndex* index_create(void){
    InvertedIndex *idx = (InvertedIndex*)calloc(1, sizeof(InvertedIndex));
    idx->map = hm_create(1024);
    idx->doc_cap = 64;
    idx->docs = (Document*)calloc(idx->doc_cap, sizeof(Document));
    return idx;
}

void index_free(InvertedIndex *idx){
    if(!idx) return;
    hm_free(idx->map, free_postings);
    for(size_t i=0;i<idx->doc_count;i++){
        free((char*)idx->docs[i].path);
    }
    free(idx->docs);
    free(idx);
}

int index_add_document(InvertedIndex *idx, const char *path){
    if(idx->doc_count == idx->doc_cap){
        idx->doc_cap *= 2;
        idx->docs = (Document*)realloc(idx->docs, idx->doc_cap * sizeof(Document));
    }
    int id = (int)idx->doc_count;
    idx->docs[id].doc_id = id;
    idx->docs[id].path = str_dup(path);
    idx->doc_count++;
    return id;
}

void index_add_term(InvertedIndex *idx, const char *term, int doc_id){
    PostingVec *vec = (PostingVec*)hm_get(idx->map, term);
    if(!vec){
        vec = (PostingVec*)calloc(1, sizeof(PostingVec));
        hm_put(idx->map, term, vec, NULL);
    }
    // Check if last posting matches doc_id
    if(vec->count > 0 && vec->data[vec->count-1].doc_id == doc_id){
        vec->data[vec->count-1].tf += 1;
        return;
    }
    if(vec->count == vec->cap){
        vec->cap = vec->cap ? vec->cap*2 : 4;
        vec->data = (Posting*)realloc(vec->data, vec->cap * sizeof(Posting));
    }
    vec->data[vec->count].doc_id = doc_id;
    vec->data[vec->count].tf = 1;
    vec->count++;
}

bool index_get_postings(InvertedIndex *idx, const char *term, const Posting **out_list, size_t *out_count){
    PostingVec *vec = (PostingVec*)hm_get(idx->map, term);
    if(!vec) return false;
    *out_list = vec->data;
    *out_count = vec->count;
    return true;
}

const Document* index_get_doc(InvertedIndex *idx, int doc_id){
    if(doc_id < 0 || (size_t)doc_id >= idx->doc_count) return NULL;
    return &idx->docs[doc_id];
}
