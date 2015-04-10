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

int binSearch(int val, int winner, int n, int *arr) {
	int low = 0, high = n-1, mid;
	while (low <= high) {
		mid = (low+high+1)/2;
		if (val < arr[mid] || winner && val <= arr[mid]) {
			high = mid-1;
		} else {
			low = mid+1;
		}
	}
	return low-1;
}

void serialRank(int n, int winner, int *rank, int sa, int ea, int *a, int sb, int *b) {
	int i = sa, j = sb;
	while (i < ea && i < n) {
		if (j < n) {
			if (a[i] < b[j] || winner && a[i] <= b[j]) {
				rank[i++] = j;
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
	blocks = bits ? (n+bits-1)/bits : 1;

	#pragma omp parallel for private(i)
	for (i=0;i<blocks;++i) {
		int loc = bits*i;
		ai[loc] = binSearch(av[loc], 0, n, bv)+1;
		bi[loc] = binSearch(bv[loc], 1, n, av)+1;
	}

	#pragma omp parallel for private(i)
	for (i=0;i<blocks;++i) {
		serialRank(n, 0, ai, bits*i, bits*(i+1), av, ai[bits*i], bv);
		serialRank(n, 1, bi, bits*i, bits*(i+1), bv, bi[bits*i], av);
	}
}

void merge(int n, int *c, int *a, int *b) {
	int *ai, *bi, i;

	ai = calloc(n, sizeof(int));
	bi = calloc(n, sizeof(int));

	rank(n, ai, bi, a, b);

	#pragma omp parallel for private(i)
	for (i=0;i<n;++i) {
		c[i+ai[i]] = a[i];
		c[i+bi[i]] = b[i];
	}

	free(ai);
	free(bi);
}

void mergeSort(int n, int *in) {
	int *a, *b, *tmp, i, size;

	a = in;
	b = calloc(n, sizeof(int));

	for (size=1;size<n;size<<=1) {
		#pragma omp parallel for private(i)
		for (i=0;i<n;i+=2*size) {
			merge(size, b+i, a+i, a+i+size);
		}
		tmp = a;
		a = b;
		b = tmp;
	}

	if (a != in) {
		#pragma omp parallel for private(i)
		for (i=0;i<n;++i) {
			in[i] = a[i];
		}
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
	int a[16] = {5, 234, 1, 23, 54, 3, 2, 12, 4, 15, 23, 12, 56, 46, 92, 17};

	display(16, a);
	mergeSort(16, a);
	display(16, a);
}

