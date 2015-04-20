#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

void display(int size, int *arr) {
	int i;
	for (i=0;i<size;++i) {
		printf("%3d%s", arr[i], (i & 0x7) == 0x7 ? "\n" : " ");
	}
	if (size & 0x7) {
		printf("\n");
	}
}

int combine(int bits, int *data, int (*f)(int, int)) {
	int i, level, N, *arr;
	N = 1 << bits;
	arr = calloc(2*N, sizeof(int));
	
	# pragma omp for private(i)
	for (i=0;i<N;++i) {
		arr[N+i] = data[i];
	}

	for (level=bits-1;level>=0;--level) {
		int levelSize = 1 << level;
		# pragma omp for private(i)
		for (i=0;i<levelSize;++i) {
			int loc = levelSize + i;
			arr[loc] = f(arr[2*loc], arr[2*loc+1]);
		}
	}

	return arr[1];
}

void prefix(int bits, int *data, int (*f)(int, int)) {
	int i, level, N, *arr;
	N = 1 << bits;
	arr = calloc(2*N, sizeof(int));
	
	# pragma omp for private(i)
	for (i=0;i<N;++i) {
		arr[N+i] = data[i];
	}

	for (level=bits-1;level>=0;--level) {
		int levelSize = 1 << level;
		# pragma omp for private(i)
		for (i=0;i<levelSize;++i) {
			int loc = levelSize + i;
			arr[loc] = f(arr[2*loc], arr[2*loc+1]);
		}
	}

	for (level=1;level<=bits;++level) {
		int levelSize = 1 << level;
		# pragma omp for private(i)
		for (i=1;i<levelSize;++i) { // Note starting at 1 (skip leftmost)
			int loc = levelSize + i;
			if (i & 1) {
				arr[loc] = arr[loc/2];
			} else {
				arr[loc] = f(arr[loc], arr[loc/2-1]);
			}
		}
	}

	# pragma omp for private(i)
	for (i=0;i<N;++i) {
		data[i] = arr[N+i];
	}
}

int intSum(int a, int b) {
	return a + b;
}

int intMin(int a, int b) {
	return a < b ? a : b;
}

int intMax(int a, int b) {
	return a > b ? a : b;
}

int compact(int *packed, int bits, int *used, int *values) {
	int n, i, *arr, cn;

	n = 1 << bits;

	arr = calloc(n, sizeof(int));
	memcpy(arr, used, n*sizeof(int));
	prefix(bits, arr, intSum);

	cn = arr[n-1];

	# pragma omp for private(i)
	for (i=0;i<n;++i) {
		if (used[i]) {
			packed[arr[i]-1] = values[i];
		}
	}

	return cn;
}

void nearestOne(int bits, int *used, int *nearests) {
	int level, n, i, *arr;

	n = 1 << bits;
	
	# pragma omp for private(i)
	for (i=0;i<n;++i) {
		nearests[i] = used[i] ? i : -1;
	}

	prefix(bits, nearests, intMax);
}

int main(int argc, char **argv) {
	int data[8] = { 1, 2, 1, 3, 5, 2, 1, 2 };
	int used[8] = { 1, 0, 0, 1, 0, 1, 1, 0 };
	int packed[8];
	int nearest[8];

	int psize = compact(packed, 3, used, data);

	printf("Data\n");
	display(8, data);
	printf("Chosen\n");
	display(8, used);

	printf("Compacted\n");
	display(psize, packed);

	nearestOne(3, used, nearest);

	printf("Nearest one\n");
	display(8, nearest);

	prefix(3, data, intMax);

	printf("Prefix max\n");
	display(8, data);
}

