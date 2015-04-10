#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

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

int binSearch(int val, int n, int *arr) {
	int low = 0, high = n-1, mid;
	printf("Bin search %d\n", val);
	display(n, arr);
	while (low <= high) {
		mid = (low+high+1)/2;
		printf("%d|%d|%d %d\n", low, mid, high, arr[mid]);
		if (val < arr[mid]) {
			high = mid-1;
		} else {
			low = mid+1;
		}
	}
	printf("%d|%d --> %d\n", low, high, low-1);
	return low-1;
}

void serialRank(int n, int *rank, int sa, int ea, int *a, int sb, int *b) {
	int i = sa, j = sb;
	while (i < ea && i < n) {
		if (j < n) {
			if (a[i] < b[j]) {
				rank[i++] = j-1;
			} else {
				j++;
			}
		} else {
			rank[i++] = j;
		}
	}
}

void rank(int n, int *ai, int *bi, int *av, int *bv) { // Ensure n is power of 2
	int bits, blocks, i;

	bits = bitLog(n-1);
	blocks = (n+bits-1)/bits;

	for (i=0;i<blocks;++i) {
		int loc = bits*i;
		ai[loc] = binSearch(av[loc], n, bv);
		bi[loc] = binSearch(bv[loc], n, av);
	}

	for (i=0;i<blocks;++i) {
		serialRank(n, ai, bits*i, bits*(i+1), av, ai[bits*i]+1, bv);
		serialRank(n, bi, bits*i, bits*(i+1), bv, bi[bits*i]+1, av);
	}
}

void merge(int n, int *c, int *a, int *b) {
	int *ai, *bi, i;

	ai = calloc(n, sizeof(int));
	bi = calloc(n, sizeof(int));

	rank(n, ai, bi, a, b);

	for (i=0;i<n;++i) {
		c[i+ai[i]+1] = a[i];
		c[i+bi[i]+1] = b[i];
	}

	free(ai);
	free(bi);
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
	int c[32], i;
	int ai[16], bi[16];

	int a[16] = {0, 4, 5, 8, 12, 13, 14, 15, 17, 18, 19, 23, 24, 26, 27, 31};
	int b[16] = {1, 2, 3, 6, 7, 9, 10, 11, 16, 20, 21, 22, 25, 28, 29, 30};

	printf("A\n");
	display(16, a);
	printf("B\n");
	display(16, b);

	rank(16, ai, bi, a, b);

	printf("A rank\n");
	display(16, ai);
	printf("B rank\n");
	display(16, bi);

	merge(16, c, a, b);

	printf("C\n");
	display(32, c);
}

