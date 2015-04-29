//
// Torbert, 29 April 2015
//
#include <xmtc.h>
#include <xmtio.h>
//
#include "poplar.h"
//
int main(void) {
    printf( "%d\n" , kmer_dim0_size ) ;
    printf( "%d\n" , kmer_dim1_size ) ;
    printf( "%d\n" , kmer[0][0]     ) ;
    printf("\n");
    //
    return 0;
}
//
// xmtcc -o kmerDemo poplar.xbo kmerDemo.c
// xmtsim -cycle -binload kmerDemo.b kmerDemo.sim
//
