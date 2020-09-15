#include <stdio.h>
// #include <stdint.h>
// #include <stdlib.h>

#include "board.h"

// #define U8 uint8_t
// #define U64 uint64_t

// #define KRED  "\x1B[31m"
// #define KNRM  "\x1B[0m"
// #define KYEL "\x1B[33m"

const U64 FILEA = 0x0101010101010101;
const U64 FILEH = 0x8080808080808080;
const U64 RANK4 = 0x00000000FF000000;
const U64 RANK5 = 0x000000FF00000000;


char* startfen = "r1b1k1nr/ppppq1pp/2n2p2/1Bb1p3/3PP3/2P2N2/PP3PPP/RNBQ1RK1 b kq d3 0 6";

int main(){
    board b = bfromfen(startfen);
    fprettyprintboard(stdout,&b);
    U64 BPIECES = 0;
    for (piece p = BPAWN; p <= BKING; ++p){
        BPIECES |= b.b[p];
    }
    fprettyprintbits(stdout,BPIECES);
    fprettyprintbits(stdout,b.b[WPAWN] << 9 & ~FILEA & BPIECES);
    return 0;
}