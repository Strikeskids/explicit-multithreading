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
                int search, low, high, mid, winner;
                search = src[block];
                low = 0;
                high = groupSize-1;
                winner = group & 1;
                
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

