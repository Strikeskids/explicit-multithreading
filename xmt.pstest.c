#include <xmtc.h>
#include <xmtio.h>

psBaseReg myRegister;

void display(int size, int *arr) {
	int i;
	for (i=0;i<size;++i) {
		printf("%3d", arr[i]);
        printf((i & 0x7) == 0x7 ? "\n" : " ");
	}
	if (size & 0x7) {
		printf("\n");
	}
}

int main(void) {
    int locs[800];

    myRegister = 0;

    spawn(0, 799) {
        int inc = 1;
        ps(inc, myRegister);
        locs[$] = inc;
    }

    // display(300, locs);

    return 0;
}

