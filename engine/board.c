#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "board.h"

#define KRED  "\x1B[31m"
#define KNRM  "\x1B[0m"
#define KYEL "\x1B[33m"

const U64 FILEA = 0x0101010101010101;
const U64 FILEH = 0x8080808080808080;
const U64 RANK4 = 0x00000000FF000000;
const U64 RANK5 = 0x000000FF00000000;

// typedef struct board {
//  U64 b[12];
// } board;

// typedef enum {WPAWN, WROOK, WKNIGHT, WBISHOP, WQUEEN, WKING,
//               BPAWN, BROOK, BKNIGHT, BBISHOP, BQUEEN, BKING} piece;

piece pfromc(char c){
    switch(c){
        case 'P' : return WPAWN;
        case 'R' : return WROOK;
        case 'N' : return WKNIGHT;
        case 'B' : return WBISHOP;
        case 'Q' : return WQUEEN;
        case 'K' : return WKING;
        case 'p' : return BPAWN;
        case 'r' : return BROOK;
        case 'n' : return BKNIGHT;
        case 'b' : return BBISHOP;
        case 'q' : return BQUEEN;
        case 'k' : return BKING;
        default : 
            fprintf(stderr,"Error, tried to convert bad char %c to piece", c);
            exit(1);
    }
}

char cfromp(piece p){
    static char* charcodes = "PRNBWKprnbwk";
    return charcodes[p];
}

board bfromfen(char* fenstring){

    board b;
    for (piece p = WPAWN; p <= BKING; ++p){
        b.b[p] = 0;
    }

    char c;
    U8 charstraversed = 0;

    for (U8 r = 7; r != 0xFF ; --r) {
        for (U8 f = 0; f < 8; ++f) {
            c = fenstring[charstraversed++];
            if (c <= '8' && c >= '1') {
                f += c - '1';
                continue;
            }
            b.b[pfromc(c)] |= (U64)1 << 8*r + f;
        }
        c = fenstring[charstraversed++];
        if ( c != ( (r != 0) ? '/' : ' ')) {
            fprintf(stderr, "Error, FEN rank not terminated,"
                    " or longer than expected, read char %c", c);
            exit(1);
        }
    }
    return b;
}

void fprettyprintboard(FILE* stream, board* b) {

    fprintf(stream, KYEL"╔═════════════════╗\n"KNRM);
    for (U8 r = 7; r != 0xFF; --r) {
        fprintf(stream, KYEL "║" KNRM);
        for (U8 f = 0; f < 8; ++f) {
            char c = ' ';
            for(piece p = WPAWN; p <= BKING; ++p){
                if(b->b[p] & (U64)1 << 8*r + f){
                    c = cfromp(p);
                    break;
                }
            }
            fprintf(stream, " %s%c"KNRM, c > 'a'? KRED:KNRM, c);
        }
        fprintf(stream, KYEL" ║  %c\n"KNRM, r + '1');
    }
    fprintf(stream, KYEL"╚═════════════════╝\n  a b c d e f g h"KNRM"\n");
}

void fprettyprintbits(FILE* stream, U64 bits) {

    fprintf(stream, KYEL"╔═════════════════╗\n"KNRM);
    for (U8 r = 7; r != 0xFF; --r) {
        fprintf(stream, KYEL "║" KNRM);
        for (U8 f = 0; f < 8; ++f) {
            fprintf(stream, KRED" %c"KNRM, bits & (U64)1 << 8*r + f? '*':' ');
        }
        fprintf(stream, KYEL" ║  %c\n"KNRM, r + '1');
    }
    fprintf(stream, KYEL"╚═════════════════╝\n  a b c d e f g h"KNRM"\n");
}



char* startfen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";