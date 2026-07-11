
#ifndef FILEWALKER_H
#define FILEWALKER_H
#include <stdbool.h>
#include "index.h"

// Walk root directory recursively, indexing any .txt files encountered.
// Returns true on success.
bool walk_and_index(const char *root, InvertedIndex *idx);

#endif
