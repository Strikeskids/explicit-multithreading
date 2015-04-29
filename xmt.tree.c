#include <xmtc.h>

void display(int size, int *arr) {
	int i;
	for (i=0;i<size;++i) {
		printf("%3d%s", arr[i], (i & 0x7) == 0x7 ? "\n" : " ");
	}
	if (size & 0x7) {
		printf("\n");
	}
}


void prefix(int bits, int *data, int *arr) {
    int n, level;

    n = 1 << bits;

    spawn(0, n-1) {
        arr[n+$] = data[$];
        arr[$] = $;
    }

    for (level=bits-1;level>=0;--level) {
        int levelSize = 1 << level;
        spawn(0, levelSize-1) {
            int loc = levelSize + $;
            arr[loc] = arr[2*loc] + arr[2*loc+1];
        }
    }

    for (level=1;level<=bits;++level) {
        int levelSize = 1 << level;
        spawn(1, levelSize-1) {
            int loc = levelSize + $;
            if ($ & 1) {
                arr[loc] = arr[loc / 2];
            } else {
                arr[loc] = arr[loc] + arr[loc/2 - 1];
            }
        }
    }

    spawn(0, n-1) {
        data[$] = arr[n+$];
    }
}

int main(int argc, char **argv) {
	int data[8] = { 1, 2, 1, 3, 5, 2, 1, 2 };
    int tmp[16];
	int used[8] = { 1, 0, 0, 1, 0, 1, 1, 0 };
	int packed[8];
	int nearest[8];

	printf("Data\n");
	display(8, data);

//	int psize = compact(packed, 3, used, data);
//
//	printf("Chosen\n");
//	display(8, used);
//
//	printf("Compacted\n");
//	display(psize, packed);
//
//	nearestOne(3, used, nearest);
//
//	printf("Nearest one\n");
//	display(8, nearest);

	prefix(3, data, tmp);

	printf("Prefix sum\n");
	display(8, data);

    return 0;
}

