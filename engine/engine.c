#include <stdio.h>
// #include <stdint.h>
// #include <stdlib.h>

#include "board.h"

// #define U8 uint8_t
// #define U64 uint64_t

// #define KRED  "\x1B[31m"
// #define KNRM  "\x1B[0m"
// #define KYEL "\x1B[33m"

// const U64 FILEA = 0x0101010101010101;
// const U64 FILEH = 0x8080808080808080;
// const U64 RANK4 = 0x00000000FF000000;
// const U64 RANK5 = 0x000000FF00000000;


// char* startfen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

int main(){
    board b = bfromfen(startfen);
    fprettyprintboard(stdout,&b);
    fprettyprintbits(stdout,b.b[0]);
    return 0;
}