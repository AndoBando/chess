//
// Created by andy on 9/27/2020.
//

#ifndef CHESSENGINE_BOARD_H
#define CHESSENGINE_BOARD_H

#include <cstdint>
#include <array>
#include <stdexcept>
#include <iostream>


typedef uint64_t U64;
typedef uint8_t U8;
typedef uint8_t square;

typedef enum {WPAWN, WROOK, WKNIGHT, WBISHOP, WQUEEN, WKING,
              BPAWN, BROOK, BKNIGHT, BBISHOP, BQUEEN, BKING} piece;

template<typename function>
void forpieces(function f){
  int p_;
  for(piece p = WPAWN; p <= BKING; p_ = (int)p, p_++, p = (piece)p_){
    f(p);
  }
}

piece pfromc(char c);

typedef struct board : std::array<U64,12> {
  void clearboard();
  board();
  explicit board(const std::string& fenstring);
} board;

U64 onehot(U8 r, U8 f);
U64 onehot(square s);

board bfromfen(const std::string& fenstring);

std::ostream& operator<<(std::ostream& os, const board& b);

/* used only for the operator<< overload */
struct bbits{
  U64 bits;
};


std::ostream& operator<<(std::ostream& os, const bbits& bb);

U64 reflectH(const U64& bits);
U64 reflectV(const U64& bits);

#endif //CHESSENGINE_BOARD_H
