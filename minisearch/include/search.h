
#ifndef SEARCH_H
#define SEARCH_H
#include <stddef.h>
#include "index.h"

typedef struct {
    int doc_id;
    double score;
} SearchHit;

// Query syntax: terms separated by spaces imply AND.
// The keyword OR (uppercase) toggles to OR between terms/groups.
// Example: "memory pointer OR stack" => (memory AND pointer) OR stack
size_t search_query(InvertedIndex *idx, const char *query, SearchHit **out_hits);

#endif
