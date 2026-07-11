
#include "tokenizer.h"
#include <ctype.h>
#include <string.h>

size_t tokenize(char *text, char **tokens, size_t max_tokens){
    size_t count = 0;
    int in_token = 0;
    for(char *p = text; *p; ++p){
        unsigned char c = (unsigned char)*p;
        if (isalnum(c)) {
            *p = (char)tolower(c);
            if (!in_token) {
                if (count < max_tokens) tokens[count] = p;
                count++;
                in_token = 1;
            }
        } else {
            *p = ' ';
            in_token = 0;
        }
    }
    // Null-terminate tokens by replacing spaces (done by parser naturally).
    // Caller should split by scanning tokens[count] until next space.
    // For convenience, we will insert null-terminators now.
    size_t finalized = 0;
    for(size_t i=0;i<count && i<max_tokens;i++){
        char *t = tokens[i];
        // advance until space or 0, then put 0
        while(*t && *t!=' ') t++;
        if(*t==' ') *t='\0';
        finalized++;
    }
    return finalized;
}
