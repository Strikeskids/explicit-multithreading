#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int data[8] = { 1, 2, 1, 3, 5, 2, 1, 2 };

void display(int size, int *arr) {
	int i;
	for (i=0;i<size;++i) {
		printf("%3d%s", arr[i], (i & 0x7) == 0x7 ? "\n" : " ");
	}
	if (!(size & 0x7)) {
		printf("\n");
	}
}

void prefix(int bits, int *data, int (*f)(int, int)) {
	int i, level, N, *arr;
	N = 1 << bits;
	arr = calloc(2*N, sizeof(int));
	
	# pragma omp for private(i)
	for (i=0;i<N;++i) {
		arr[N+i] = data[i];
	}

	display(2*N, arr);

	for (level=bits-1;level>=0;--level) {
		int levelSize = 1 << level;
		# pragma omp for private(i)
		for (i=0;i<levelSize;++i) {
			int loc = levelSize + i;
			arr[loc] = f(arr[2*loc], arr[2*loc+1]);
		}
	}

	display(2*N, arr);

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

	display(2*N, arr);

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

int main(int argc, char **argv) {
	prefix(3, data, intSum);

	display(8, data);
}

