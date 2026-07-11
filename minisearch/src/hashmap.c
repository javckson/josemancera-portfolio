
#include "hashmap.h"
#include "strutil.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct Node {
    char *key;
    void *val;
    struct Node *next;
} Node;

struct HashMap {
    Node **buckets;
    size_t cap;
    size_t size;
};

static uint64_t hash_str(const char *s){
    // FNV-1a 64-bit
    uint64_t h = 1469598103934665603ULL;
    while(*s){
        h ^= (unsigned char)*s++;
        h *= 1099511628211ULL;
    }
    return h;
}

static HashMap* hm_rehash(HashMap *hm){
    size_t newcap = hm->cap * 2;
    Node **nb = (Node**)calloc(newcap, sizeof(Node*));
    for(size_t i=0;i<hm->cap;i++){
        Node *n = hm->buckets[i];
        while(n){
            Node *next = n->next;
            uint64_t h = hash_str(n->key);
            size_t idx = (size_t)(h % newcap);
            n->next = nb[idx];
            nb[idx] = n;
            n = next;
        }
    }
    free(hm->buckets);
    hm->buckets = nb;
    hm->cap = newcap;
    return hm;
}

HashMap* hm_create(size_t initial_capacity){
    if (initial_capacity < 16) initial_capacity = 16;
    HashMap *hm = (HashMap*)calloc(1, sizeof(HashMap));
    hm->cap = initial_capacity;
    hm->buckets = (Node**)calloc(hm->cap, sizeof(Node*));
    return hm;
}

void hm_free(HashMap *hm, void (*free_value)(void*)){
    if(!hm) return;
    for(size_t i=0;i<hm->cap;i++){
        Node *n = hm->buckets[i];
        while(n){
            Node *next = n->next;
            if(free_value) free_value(n->val);
            free(n->key);
            free(n);
            n = next;
        }
    }
    free(hm->buckets);
    free(hm);
}

void* hm_put(HashMap *hm, const char *key, void *value, bool *inserted){
    if ((hm->size * 10) / hm->cap > 7) hm_rehash(hm); // load factor ~0.7
    uint64_t h = hash_str(key);
    size_t idx = (size_t)(h % hm->cap);
    for(Node *n = hm->buckets[idx]; n; n = n->next){
        if(strcmp(n->key, key)==0){
            if(inserted) *inserted = false;
            return n->val;
        }
    }
    Node *nn = (Node*)calloc(1, sizeof(Node));
    nn->key = str_dup(key);
    nn->val = value;
    nn->next = hm->buckets[idx];
    hm->buckets[idx] = nn;
    hm->size++;
    if(inserted) *inserted = true;
    return value;
}

void* hm_get(HashMap *hm, const char *key){
    uint64_t h = hash_str(key);
    size_t idx = (size_t)(h % hm->cap);
    for(Node *n = hm->buckets[idx]; n; n = n->next){
        if(strcmp(n->key, key)==0) return n->val;
    }
    return NULL;
}

void hm_foreach(HashMap *hm, hm_iter_cb cb, void *user){
    for(size_t i=0;i<hm->cap;i++){
        for(Node *n = hm->buckets[i]; n; n = n->next){
            cb(n->key, n->val, user);
        }
    }
}

size_t hm_size(HashMap *hm){ return hm->size; }
