
#ifndef TOKENIZER_H
#define TOKENIZER_H
#include <stddef.h>

// In-place tokenize: replaces non-alnum with space, lowercases ASCII letters.
// Returns count of tokens written to `tokens` (array of char*), up to max_tokens.
size_t tokenize(char *mutable_text, char **tokens, size_t max_tokens);

#endif
