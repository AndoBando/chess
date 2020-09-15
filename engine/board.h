#include <stdint.h>

#define U8 uint8_t
#define U64 uint64_t

typedef struct board {
 U64 b[12];
} board;

typedef enum {WPAWN, WROOK, WKNIGHT, WBISHOP, WQUEEN, WKING,
              BPAWN, BROOK, BKNIGHT, BBISHOP, BQUEEN, BKING} piece;

piece pfromc(char c);
char cfromp(piece p);
board bfromfen(char* fenstring);
void fprettyprintboard(FILE* stream, board* b);
void fprettyprintbits(FILE* stream, U64 bits);
char* startfen;