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
		for (i=2*levelSize-1;i>=levelSize;--i) {
			arr[i] = f(arr[2*i], arr[2*i+1]);
		}
	}

	display(2*N, arr);

	for (level=1;level<=bits;++level) {
		int levelSize = 1 << level;
		# pragma omp for private(i)
		for (i=2*levelSize-1;i>levelSize;--i) { // Note > not >= (we want to skip leftmost)
			if (i & 1) {
				arr[i] = arr[i/2];
			} else {
				arr[i] = f(arr[i], arr[i/2 - 1]);
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

