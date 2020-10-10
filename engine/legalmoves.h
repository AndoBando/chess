//
// Created by andy on 9/28/2020.
//

#ifndef CHESSENGINE_LEGALMOVES_H
#define CHESSENGINE_LEGALMOVES_H

#include <cstdint>
#include <iostream>
#include "board.h"

template<typename function>
void forhotsquares(U64 q, function f) {
  square i;
  while (q != 0) {
    i = __builtin_ctzl(q);
    q ^= onehot(i);
    f(i);
  }
}

typedef struct move{
  square from;
  square to;
} move;

std::ostream& operator<<(std::ostream& os, const move& m);

void setmasks(const board& b);

void WPmoves(const board& b);
void WNmoves(const board& b);
void WRmoves(const board& b);

#endif //CHESSENGINE_LEGALMOVES_H
