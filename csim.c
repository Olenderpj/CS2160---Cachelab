#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include "cachelab.h"

const int m = sizeof(long) * 8;
int hitCount = 0, missCount = 0, evictCount = 0;
int vFlag = 0, hFlag = 0;
int s, b, E, t;
char *trace;


struct Line
{
    int valid ;
    unsigned tag ;
    struct Line *next;
};

struct Set
{
    int E;
    struct Line *headLine;
    struct Set *next;
};

struct Cache
{
    int S;
    struct Set *headSet;
};

// return the number of valid lines in a set
int set_size(struct Set *set)
{
    struct Line *curr = set->headLine;
    int size = 0;
    while (curr != NULL){
        ++size;
        curr = curr->next;
    }
    return size;
}

// initialize set with capacity E and no valid lines
void init_set(struct Set *set, int E){
    set->E = E;
    set->headLine = NULL;
    set->next = NULL;
}

// initialize cache with 2^s empty sets
void init_cache(struct Cache *cache, int s, int E) {
    cache->S = (1 << s);
    struct Set *thisset = (struct Set*)malloc(sizeof(struct Set));
    init_set(thisset, E);
    cache->headSet= thisset;
    int i = 1;
    for (i ; i < cache->S; ++i){
        struct Set *nextset = (struct Set*)malloc(sizeof(struct Set));
        init_set(nextset, E);
        thisset->next = nextset;
        thisset = nextset;
    }
}

// delete the last line of a set
void delete_last_line(struct Set *set){
    struct Line *curr = set->headLine;
    struct Line *prev = NULL;
    struct Line *prev2 = NULL;
    while (curr != NULL){
        prev2 = prev;
        prev = curr;
        curr = curr->next;
    }
    if (prev2 != NULL){
        prev2->next = NULL;
    } else {
        set->headLine = NULL;
    }
    if (prev != NULL) free(prev);
}

// add a (valid) line to the head of a set, evict when exceeds capacity
void add_line_to_head(struct Set *set, struct Line *line){
    if (set_size(set) == set->E){
        delete_last_line(set);
        evictCount++;
        if (vFlag) printf("eviction ");
    }
    line->next = set->headLine;
    set->headLine = line;
}

// move the matched line to head of the set
void move_line_to_head(struct Set *set, struct Line *line, struct Line *prev){
    if (prev != NULL){
        prev->next = line->next;
        line->next = set->headLine;
        set->headLine = line;
    }
}

// the procedure of fetching an address in the cache
void access_cache(struct Cache *cache, unsigned address){
    int tag_bits = address >> (s+b);
    int set_bits = (address << t) >> (t+b);

    // move to target_set according to set_bits
    struct Set *target_set = cache->headSet;
    int i;
    for (i = 0; i < set_bits; ++i){
        target_set = target_set->next;
    }

    struct Line *ln = target_set->headLine;
    struct Line *prev = NULL;
    while (ln != NULL){
        // if there is a  hit
        if (ln->valid && (ln->tag == tag_bits)){
            hitCount++;
            if (vFlag) printf("hit ");
            move_line_to_head(target_set, ln, prev);
            return ;
        }
        prev = ln;
        ln = ln->next;
    }

    // if there is no match then miss and add a new line
    missCount++;
    if (vFlag) printf("miss ");
    struct Line *newln = (struct Line *)malloc(sizeof(struct Line));
    newln->valid = 1;
    newln->tag = tag_bits;
    add_line_to_head(target_set,newln);
}

// free cache, every set of cache, and every line in set
void free_cache(struct Cache *cache){
    struct Set *set_to_free = cache->headSet;
    while (!set_to_free){
        struct Line *ln_to_free = set_to_free->headLine;
        while (!ln_to_free){
            struct Line *temp_ln = ln_to_free->next;
            free(ln_to_free);
            ln_to_free = temp_ln;
        }
        struct Set *temp_set = set_to_free->next;
        free(set_to_free);
        set_to_free = temp_set;
    }
    free(cache);
}

int main(int argc, char** argv){
    int opt;
    char op;
    unsigned addr;
    int size;

    // parse flag commands
    while(-1 != (opt = getopt(argc, argv, "vhs:E:b:t:"))) {
        switch(opt) {
            case 'v':
                vFlag = 1;
                break;
            case 'h':
                hFlag = 1;
                break;
            case 's':
                s = atoi(optarg);
                if(s < 0 && s > m ){
                    printf("Error: s value must be in [0, word_length].\n");
                    exit(-1);
                }
                break;
            case 'E':
                E = atoi(optarg);
                if(E <= 0){
                    printf("Error: E value must be larger than 0.\n");
                    exit(-1);
                }
                break;
            case 'b':
                b = atoi(optarg);
                if(b < 0 || b > m){
                    printf("Error: b value must be in [0, word_length].\n");
                    exit(-1);
                }
                break;
            case 't':
                trace = optarg;
                break;
            default:
                break;
        }
    }

    t = m - s - b;
    struct Cache *mycache = (struct Cache *)malloc(sizeof(struct Cache));
    init_cache(mycache, s, E);

    // read trace file and access cache
    FILE *traceFile;
    traceFile = fopen(trace, "r");
    if (!traceFile){
        fprintf(stderr, "Error: Trace file cannot be opened.\n");
        return -1;
    }
    // scan the trace file for the flag
    while(fscanf(traceFile, "%c %x, %d", &op, &addr, &size) > 0) {
        if (vFlag) printf("%c %x, %d ", op, addr, size);
        switch(op){
            case 'L':
                access_cache(mycache, addr);
                break;
            case 'S':
                access_cache(mycache, addr);
                break;
            case 'M':
                access_cache(mycache, addr);
                access_cache(mycache, addr);
                break;
        }
        if (vFlag) printf("\n");
    }
    fclose(traceFile);

    free_cache(mycache);
    printSummary(hitCount, missCount, evictCount);

    return 0;
}