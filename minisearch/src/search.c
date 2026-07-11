
#include "search.h"
#include "tokenizer.h"
#include "strutil.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef struct {
    int doc_id;
    double score;
} Score;

static int cmp_score_desc(const void *a, const void *b){
    const Score *x = (const Score*)a, *y = (const Score*)b;
    if (y->score > x->score) return 1;
    if (y->score < x->score) return -1;
    return x->doc_id - y->doc_id;
}

static void add_or_increment(Score **arr, size_t *count, size_t *cap, int doc_id, double delta){
    for(size_t i=0;i<*count;i++){
        if((*arr)[i].doc_id == doc_id){ (*arr)[i].score += delta; return; }
    }
    if(*count == *cap){
        *cap = *cap ? *cap*2 : 16;
        *arr = (Score*)realloc(*arr, *cap * sizeof(Score));
    }
    (*arr)[*count].doc_id = doc_id;
    (*arr)[*count].score = delta;
    (*count)++;
}

// A simple parser/evaluator for AND/OR.
size_t search_query(InvertedIndex *idx, const char *query, SearchHit **out_hits){
    if(!query || !*query){ *out_hits=NULL; return 0; }
    char *q = str_dup(query);
    // Tokenize by spaces, handling OR as operator.
    size_t max = 1024;
    char **tokens = (char**)malloc(max*sizeof(char*));
    size_t n=0;
    for(char *p=strtok(q, " "); p; p=strtok(NULL, " ")){
        if(n<max) tokens[n]=p;
        n++;
    }
    if(n > max) n = max; // clamp: only `max` slots were ever written to `tokens`
    // Evaluate left-to-right: groups separated by OR are unioned; inside group, intersect.
    Score *scores = NULL; size_t scount=0, scap=0;
    // Current group intersection working set
    int *work = NULL; size_t wcount=0, wcap=0;
    // Once a term in the current AND-group has zero postings, the whole group
    // is empty. group_dead keeps it that way until the next OR resets it —
    // otherwise the following term would wrongly re-seed `work` from scratch.
    bool group_dead = false;
    for(size_t i=0;i<n;i++){
        if(strcmp(tokens[i], "OR")==0){
            // flush current group into scores
            for(size_t k=0;k<wcount;k++) add_or_increment(&scores,&scount,&scap, work[k], 1.0);
            wcount = 0;
            group_dead = false;
            continue;
        }
        // term: lowercase
        char *term = tokens[i];
        for(char *p=term; *p; ++p) *p = (char)tolower((unsigned char)*p);
        if(group_dead) continue; // this group is already known-empty; skip to next OR
        // postings
        const Posting *plist=NULL; size_t pc=0;
        if(!index_get_postings(idx, term, &plist, &pc)){
            // term not found; the whole AND-group is now empty
            free(work); work = NULL; wcap = 0;
            wcount = 0;
            group_dead = true;
            continue;
        }
        // intersect with work
        if(wcount==0){
            // initialize with doc_ids from plist
            wcap = pc; work = (int*)realloc(work, wcap*sizeof(int));
            for(size_t j=0;j<pc;j++) work[j] = plist[j].doc_id;
            wcount = pc;
        } else {
            size_t newc=0;
            int *tmp = (int*)malloc(wcount * sizeof(int));
            for(size_t a=0;a<wcount;a++){
                int d = work[a];
                // check if d appears in plist
                for(size_t b=0;b<pc;b++){
                    if(plist[b].doc_id == d){ tmp[newc++] = d; break; }
                }
            }
            free(work); work = tmp; wcount = newc; wcap = newc;
        }
    }
    // flush last group
    for(size_t k=0;k<wcount;k++) add_or_increment(&scores,&scount,&scap, work[k], 1.0);
    free(work);
    free(tokens);
    free(q);

    // Convert to SearchHit and attach a naive TF-based score by summing term frequencies of matched terms.
    SearchHit *hits = NULL; size_t hcount=0, hcap=0;
    for(size_t i=0;i<scount;i++){
        int doc_id = scores[i].doc_id;
        double s = 0.0;
        // Very naive: sum over all words in query that appear in doc (counts as +tf)
        // Proper BM25/TF-IDF omitted for brevity.
        char *ql = str_to_lower_dup(query);
        char *tok = strtok(ql, " ");
        while(tok){
            if(strcmp(tok, "or")==0){ tok = strtok(NULL, " "); continue; }
            const Posting *plist=NULL; size_t pc=0;
            if(index_get_postings(idx, tok, &plist, &pc)){
                for(size_t b=0;b<pc;b++) if(plist[b].doc_id==doc_id) s += plist[b].tf;
            }
            tok = strtok(NULL, " ");
        }
        free(ql);
        if(hcount==hcap){ hcap = hcap? hcap*2 : 16; hits = (SearchHit*)realloc(hits, hcap*sizeof(SearchHit)); }
        hits[hcount].doc_id = doc_id;
        hits[hcount].score = s>0? s : 1.0;
        hcount++;
    }
    free(scores);
    // sort desc (skip if empty — hits is NULL when hcount==0, and qsort
    // isn't guaranteed safe to call with a null base pointer even at n=0)
    if (hcount > 0) {
        qsort(hits, hcount, sizeof(SearchHit), cmp_score_desc);
    }
    *out_hits = hits;
    return hcount;
}
