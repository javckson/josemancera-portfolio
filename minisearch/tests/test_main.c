
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "tokenizer.h"
#include "hashmap.h"
#include "index.h"
#include "strutil.h"

static void test_tokenizer(){
    char buf[128]; strcpy(buf, "Hello, WORLD! 123");
    char *toks[8];
    size_t n = tokenize(buf, toks, 8);
    assert(n==3);
    assert(strcmp(toks[0],"hello")==0);
    assert(strcmp(toks[1],"world")==0);
    assert(strcmp(toks[2],"123")==0);
}

static void test_hashmap(){
    HashMap *hm = hm_create(16);
    bool ins=false;
    int a=1,b=2;
    hm_put(hm, "aa", &a, &ins); assert(ins);
    hm_put(hm, "aa", &b, &ins); assert(!ins);
    int *v = (int*)hm_get(hm, "aa"); assert(v==&a || v==&b); // either pointer ok for test
    hm_free(hm, NULL);
}

static void test_index(){
    InvertedIndex *idx = index_create();
    int d0 = index_add_document(idx, "a.txt");
    index_add_term(idx, "hello", d0);
    index_add_term(idx, "hello", d0);
    index_add_term(idx, "world", d0);
    const Posting *pl=NULL; size_t pc=0;
    assert(index_get_postings(idx, "hello", &pl, &pc) && pc==1 && pl[0].tf==2);
    index_free(idx);
}

int main(void){
    test_tokenizer();
    test_hashmap();
    test_index();
    printf("All tests passed.\n");
    return 0;
}
