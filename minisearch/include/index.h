
#ifndef INDEX_H
#define INDEX_H
#include <stddef.h>
#include "hashmap.h"

typedef struct {
    int doc_id;
    int tf; // term frequency
} Posting;

typedef struct {
    int doc_id;
    const char *path; // owned by index
} Document;

typedef struct {
    HashMap *map;     // word -> vector<Posting>
    Document *docs;   // dynamic array
    size_t doc_count;
    size_t doc_cap;
} InvertedIndex;

InvertedIndex* index_create(void);
void index_free(InvertedIndex *idx);

int index_add_document(InvertedIndex *idx, const char *path);
void index_add_term(InvertedIndex *idx, const char *term, int doc_id);

// Get postings list for term (array pointer and count). Returns false if not found.
bool index_get_postings(InvertedIndex *idx, const char *term, const Posting **out_list, size_t *out_count);

const Document* index_get_doc(InvertedIndex *idx, int doc_id);

#endif
