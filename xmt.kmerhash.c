#include <xmtc.h>
#include <xmtio.h>

#include "poplar.h"

#define BUCKETS 8192
#define BUCKET_HEAP_SIZE 10000
#define FNV_PRIME_32 16777619

typedef struct hash_chain_s {
    int locked;
    int num;
    int count;
    struct hash_chain_s *next;
} hash_chain_t;

hash_chain_t buckets[BUCKETS], bucketHeap[BUCKET_HEAP_SIZE];
psBaseReg heapEnd;

#define KMER_CMP(a, b, r) {\
    int *aval = kmer[a], *bval = kmer[b], i;\
    for (i=0;i<kmer_dim1_size;++i) {\
        if (aval[i] != bval[i]) {\
            break;\
        }\
    }\
    r = i == kmer_dim1_size;\
}

#define HASH_KMER(hash, num) {\
    int i, h, *value = kmer[num];\
    hash = 0xcdbdebee;\
    for (i=0;i<kmer_dim1_size;++i) {\
        hash ^= 0xff ^ value[i];\
        hash *= FNV_PRIME_32;\
    }\
}

int runHashes(int n) {
    spawn(0, n-1) {
        int hash, lockValue, matches, heapLoc, cmpres;
        hash_chain_t *found, *end;
        HASH_KMER(hash, $);

        found = &buckets[hash & BUCKETS];
    
        lockValue = 1;
        psm(lockValue, found->locked);
        while (lockValue != 0) {
            lockValue = -1;
            psm(lockValue, found->locked);
            lockValue = 1;
            psm(lockValue, found->locked);
        }

        if (found->count == 0) {
            found->num = $;
            found->count = 1;
        } else { KMER_CMP(found->num, $, cmpres); if (cmpres) {
            found->count += 1;
        } else {
            end = found;
            matches = 0;
            while (end->next) {
                end = end->next;
                KMER_CMP(end->num, $, cmpres);
                if (cmpres) {
                    matches = 1;
                    break;
                }
            }
            if (matches) {
                end->count += 1;
            } else {
                heapLoc = 1;
                ps(heapLoc, heapEnd);
                end = end->next = &bucketHeap[heapLoc];
                end->num = $;
                end->next = 0;
                end->count = 1;
            }
        }}

        lockValue = -1;
        psm(lockValue, found->locked);
    }
}

void kmerDisp(int a) {
    int i;
    for (i=0;i<kmer_dim1_size;++i) {
        printf("%d", kmer[a][i]);
    }
    printf("\n");
}

int main(void) {
    int i;
    hash_chain_t *cur;

    heapEnd = 0;
    spawn(0, BUCKETS-1) {
        hash_chain_t *chain = &buckets[$];
        chain->locked = 0;
        chain->num = -1;
        chain->count = 0;
        chain->next = 0;
    }

    runHashes(65536);
    
    for (i=0;i<BUCKETS;++i) {
        for (cur = &buckets[i];cur && cur->count;cur=cur->next) {
            if (cur->count > 1) {
                printf("%d ", cur->count);
                kmerDisp(cur->num);
            }
        }
    }
}

/*
 * xmtcc -o xmt.kmerhash poplar.xbo xmt.kmerhash.c
 * xmtsim -cycle -binload xmt.kmerhash.b xmt.kmerhash.sim
 */ 
