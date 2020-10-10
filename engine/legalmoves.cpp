//
// Created by andy on 9/28/2020.
//

#include "legalmoves.h"

constexpr U64 FILEM[8] = {
        0x0101010101010101, 0x0202020202020202,
        0x0404040404040404, 0x0808080808080808,
        0x1010101010101010, 0x2020202020202020,
        0x4040404040404040, 0x8080808080808080,
};

constexpr U64 RANKM[8] = {
        0x00000000000000FF, 0x000000000000FF00,
        0x0000000000FF0000, 0x00000000FF000000,
        0x000000FF00000000, 0x0000FF0000000000,
        0x00FF000000000000, 0xFF00000000000000,
};

constexpr U64 DIAGM[15] = {
        0x8000000000000000, 0x4080000000000000,
        0x2040800000000000, 0x1020408000000000,
        0x0810204080000000, 0x0408102040800000,
        0x0204081020408000, 0x0102040810204080,
        0x0001020408102040, 0x0000010204081020,
        0x0000000102040810, 0x0000000001020408,
        0x0000000000010204, 0x0000000000000102,
        0x0000000000000001
};

constexpr U64 ANTIDIAGM[15] = {
        0x0100000000000000, 0x0201000000000000,
        0x0402010000000000, 0x0804020100000000,
        0x1008040201000000, 0x2010080402010000,
        0x4020100804020100, 0x8040201008040201,
        0x0080402010080402, 0x0000804020100804,
        0x0000008040201008, 0x0000000080402010,
        0x0000000000804020, 0x0000000000008040,
        0x0000000000000080
};

std::ostream& operator<<(std::ostream& os, const move& m){
  os << (char)('a' + m.from % 8) << (char)('1' + m.from / 8);
  os << " -> ";
  os << (char)('a' + m.to % 8) << (char)('1' + m.to / 8);
}

U64 BPIECES, WPIECES, PIECES, NPIECES;

void setmasks(const board& b) {
  WPIECES = b[WPAWN] | b[WROOK] | b[WKNIGHT] | b[WBISHOP] | b[WQUEEN] | b[WKING];
  BPIECES = b[BPAWN] | b[BROOK] | b[BKNIGHT] | b[BBISHOP] | b[BQUEEN] | b[BKING];
  PIECES = WPIECES | BPIECES;
  NPIECES = ~PIECES;
}

void WPmoves(const board& b) {
  printf("Pawn moves : \n");

  U64 q;
  U8 i;
  move m;

  q = b[WPAWN] << 9 & ~FILEM[0] & BPIECES;
  forhotsquares(q,[&](square i){
    m.to = i;
    m.from = i - 9;
    std::cout << m << "\n";
  });

  q = b[WPAWN] << 7 & ~FILEM[7] & BPIECES;
  forhotsquares(q,[&](square i){
    m.to = i;
    m.from = i - 7;
    std::cout << m << "\n";
  });

  q = b[WPAWN] << 8 & NPIECES;
  forhotsquares(q,[&](square i){
    m.to = i;
    m.from = i - 8;
    std::cout << m << "\n";
  });

  q = b[WPAWN] << 16 & NPIECES & RANKM[3] & NPIECES << 8;
  forhotsquares(q,[&](square i){
    m.to = i;
    m.from = i - 16;
    std::cout << m << "\n";
  });
}

void WNmoves(const board& b) {
  printf("Knight moves : \n");
  U64 q;
  U8 i;
  move m;

  q = b[WKNIGHT] << 10 & ~(FILEM[0] | FILEM[1]) & ~WPIECES ;
  forhotsquares(q,[&](square i){
    m.to = i;
    m.from = i - 10;
    std::cout << m << "\n";
  });
  q = b[WKNIGHT] << 6 & ~(FILEM[6] | FILEM[7]) & ~WPIECES ;
  forhotsquares(q,[&](square i){
    m.to = i;
    m.from = i - 6;
    std::cout << m << "\n";
  });
  q = b[WKNIGHT] << 17 & ~(FILEM[0]) & ~WPIECES ;
  forhotsquares(q,[&](square i){
    m.to = i;
    m.from = i - 17;
    std::cout << m << "\n";
  });
  q = b[WKNIGHT] << 15 & ~(FILEM[7]) & ~WPIECES ;
  forhotsquares(q,[&](square i){
    m.to = i;
    m.from = i - 15;
    std::cout << m << "\n";
  });
  q = b[WKNIGHT] >> 10 & ~(FILEM[0] | FILEM[1]) & ~WPIECES ;
  forhotsquares(q,[&](square i){
    m.to = i;
    m.from = i + 10;
    std::cout << m << "\n";
  });
  q = b[WKNIGHT] >> 6 & ~(FILEM[6] | FILEM[7]) & ~WPIECES ;
  forhotsquares(q,[&](square i){
    m.to = i;
    m.from = i + 6;
    std::cout << m << "\n";
  });
  q = b[WKNIGHT] >> 17 & ~(FILEM[0]) & ~WPIECES ;
  forhotsquares(q,[&](square i){
    m.to = i;
    m.from = i + 17;
    std::cout << m << "\n";
  });
  q = b[WKNIGHT] >> 15 & ~(FILEM[7]) & ~WPIECES ;
  forhotsquares(q,[&](square i){
    m.to = i;
    m.from = i + 15;
    std::cout << m << "\n";
  });
}

void WRmoves(const board& b){
  printf("Rook moves : \n");
  U64 q;
  U8 i;
  move m;
  forhotsquares(b[WROOK],[&](square i){
    m.from = i;
    U64 s = onehot(i);
    q = ((PIECES - 2 * s) ^ PIECES) & ~WPIECES;
  });
//  q = ((PIECES - 2 * b[WROOK]) ^ PIECES) & ~WPIECES;
//  std::cout << bbits{q};
  q = reflectH(((reflectH(PIECES) - reflectH(2 * b[WROOK])) ^ reflectH(PIECES)) & ~reflectH(WPIECES));
  std::cout << bbits{q};
}