#include <xmtc.h>
#include <xmtio.h>

void display(int size, int *arr) {
	int i;
	for (i=0;i<size;++i) {
		printf("%3d%s", arr[i], (i & 0x7) == 0x7 ? "\n" : " ");
	}
	if (size & 0x7) {
		printf("\n");
	}
}

int bitLog(int n) {
	int i = 0;
	for (i=0;n;n>>=1) {
		i++;
	}
	return i;
}

#ifdef DEBUG
#define SPAWN_SETUP int dollarSign;
#define SPAWN(start, end) for (dollarSign=start;dollarSign<=end;++dollarSign) { int $ = dollarSign;
#define SPAWN_END }
#define PRINTF_DEBUG(...) printf(__VA_ARGS__)
#else
#define SPAWN(start, end) spawn(start, end)
#define SPAWN_SETUP ;
#define SPAWN_END ;
#define PRINTF_DEBUG(...) ;
#endif

void mergeSort(int n, int *in, int *tmp, int *indices) {
    SPAWN_SETUP;
    int *source, *dest, i, groupSize;

    source = in;
    dest = tmp;

    for (groupSize=1;groupSize<n;groupSize<<=1) {
        int groupCount = n / groupSize / 2;
        int blockBits, blockCount;
        blockBits = bitLog(groupSize-1);
        blockCount = blockBits ? (groupSize+blockBits-1)/blockBits : 1;

        SPAWN(0, 2*blockCount*groupCount-1) {
            int group = $ / blockCount;
            int block = blockBits * ($ % blockCount);
            int *src, *oth;
            int *ind;
            
            PRINTF_DEBUG("BS search=%d group=%d block=%d startSearch=%d\n", group ^ 1, group, block, (group ^ 1)*groupSize);

            src = source + group * groupSize;
            oth = source + (group ^ 1) * groupSize;
            ind = indices + group * groupSize;
            
            { // Binary search for location of block pivot
                int search, low, high, mid, winner;
                search = src[block];
                low = 0;
                high = groupSize-1;
                winner = group & 1;

                PRINTF_DEBUG("BS DATA goal=%d (%d %d) %d\n", search, low, high, winner);
                
        	    while (low <= high) {
        	    	mid = (low+high+1)/2;
        	    	if (search < oth[mid] || winner && search <= oth[mid]) {
        	    		high = mid-1;
        	    	} else {
        	    		low = mid+1;
        	    	}
        	    }

                ind[block] = (low-1) + 1;
            }
        SPAWN_END }

        SPAWN(0, 2*blockCount*groupCount-1) {
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
                int loser, srcIndex, othIndex;

                loser = group & 1;

                srcIndex = srcStart;
                othIndex = ind[srcIndex];
                while (srcIndex < srcEnd) {
                    if (othIndex < groupSize) {
                        if (src[srcIndex] < oth[othIndex] || loser && src[srcIndex] == oth[othIndex]) {
                            ind[srcIndex++] = othIndex;
                        } else {
                            othIndex++;
                        }
                    } else {
                        ind[srcIndex++] = othIndex;
                    }
                }
            }
        SPAWN_END }

        SPAWN(0, 2*groupCount*groupSize-1) {
            int group = $ / groupSize;
            int groupIndex = $ % groupSize;

            int *ind, *dst, *src;

            ind = indices + group * groupSize;
            src = source + group * groupSize;
            dst = dest + (group & (~1)) * groupSize;

            dst[groupIndex + ind[groupIndex]] = src[groupIndex];
        SPAWN_END }

        tmp = source;
        source = dest;
        dest = tmp;
    }

    if (in != source) {
        SPAWN(0, n) {
            in[$] = source[$];
        SPAWN_END }
    }
}

int main(int argc, char **argv) {
	int a[16] = {5, 234, 1, 23, 54, 3, 2, 12, 4, 15, 23, 12, 56, 46, 92, 17};
    int tmp1[16], tmp2[16];

    printf("Array before sorting\n");
	display(16, a);
    printf("Start sorting\n");
	mergeSort(16, a, tmp1, tmp2);
    printf("Array after sorting\n");
    display(16, a);
    return 0;
}

