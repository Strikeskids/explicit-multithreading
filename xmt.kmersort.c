#include <xmtc.h>
#include <xmtio.h>

#include "poplar.h"

#define KMERLT(result, a, b, loser) {\
    int i;\
    int *av, *bv;\
    if (a >= kmer_dim0_size) { result = b >= kmer_dim0_size ? loser : 0; }\
    else if (b >= kmer_dim0_size) { result = 1; }\
    else {\
    av = kmer[a];\
    bv = kmer[b];\
    for (i=0;i<kmer_dim1_size;++i) {\
        if (av[i] < bv[i]) {\
            result = 1;\
            break;\
        }\
        if (av[i] > bv[i]) {\
            result = 0;\
            break;\
        }\
    }\
    if (loser && i == kmer_dim1_size) result = 1;\
    }\
}

int bitLog(int n) {
	int i = 0;
	for (i=0;n;n>>=1) {
		i++;
	}
	return i;
}

void mergeSort(int n, int *in, int *tmp, int *indices) {
    int *source, *dest, i, groupSize;

    source = in;
    dest = tmp;

    for (groupSize=1;groupSize<n;groupSize<<=1) {
        int groupCount = n / groupSize / 2;
        int blockBits, blockCount;
        blockBits = bitLog(groupSize-1);
        blockCount = blockBits ? (groupSize+blockBits-1)/blockBits : 1;

        spawn(0, 2*blockCount*groupCount-1) {
            int group = $ / blockCount;
            int block = blockBits * ($ % blockCount);
            int *src, *oth;
            int *ind;

            src = source + group * groupSize;
            oth = source + (group ^ 1) * groupSize;
            ind = indices + group * groupSize;
            
            { // Binary search for location of block pivot
                int search, low, high, mid, loser, lt;
                search = src[block];
                low = 0;
                high = groupSize-1;
                loser = group & 1;
                
        	    while (low <= high) {
        	    	mid = (low+high+1)/2;
                    KMERLT(lt, search, oth[mid], loser);
        	    	if (lt) {
        	    		high = mid-1;
        	    	} else {
        	    		low = mid+1;
        	    	}
        	    }

                ind[block] = (low-1) + 1;
            }
        }

        spawn(0, 2*blockCount*groupCount-1) {
            int group = $ / blockCount;
            int block = blockBits * ($ % blockCount);
            int nextBlock = block + blockBits;
            int srcStart, srcEnd;

            srcStart = block;
            srcEnd = nextBlock < groupSize ? nextBlock : groupSize;

            int *src, *oth;
            int *ind;

            ind = indices + group * groupSize;
            src = source + group * groupSize;
            oth = source + (group ^ 1) * groupSize;

            { // Serial rank
                int loser, srcIndex, othIndex, lt;

                loser = group & 1;

                srcIndex = srcStart;
                othIndex = ind[srcIndex];
                while (srcIndex < srcEnd) {
                    if (othIndex < groupSize) {
                        KMERLT(lt, src[srcIndex], oth[othIndex], loser);
                        if (lt) {
                            ind[srcIndex++] = othIndex;
                        } else {
                            othIndex++;
                        }
                    } else {
                        ind[srcIndex++] = othIndex;
                    }
                }
            }
        }

        spawn(0, 2*groupCount*groupSize-1) {
            int group = $ / groupSize;
            int groupIndex = $ % groupSize;

            int *ind, *dst, *src;

            ind = indices + group * groupSize;
            src = source + group * groupSize;
            dst = dest + (group & (~1)) * groupSize;

            dst[groupIndex + ind[groupIndex]] = src[groupIndex];
        }

        tmp = source;
        source = dest;
        dest = tmp;
    }

    if (in != source) {
        spawn(0, n) {
            in[$] = source[$];
        }
    }
}

void kmerDisp(int a) {
    int i;
    for (i=0;i<kmer_dim1_size;++i) {
        printf("%d", kmer[a][i]);
    }
    printf("\n");
}

int arr[262144], tmp[262144], tmp2[262144];

int main(int argc, char **argv) {
    printf("%d %d\n", kmer_dim0_size, kmer_dim1_size);
    int i;
    for (i=0;i<262144;++i) {
        arr[i] = i;
    }

    mergeSort(8192, arr, tmp, tmp2);

    for (i=0;i<300;++i) {
        kmerDisp(arr[i]);
    }

    return 0;
}

/*
 * xmtcc -o xmt.kmersort poplar.xbo xmt.kmersort.c
 * xmtsim -cycle -binload xmt.kmersort.b xmt.kmersort.sim
 */ 
