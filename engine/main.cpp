#include <iostream>
#include <string>
#include <chrono>
#include "board.h"
#include "legalmoves.h"

int main() {
  constexpr char fen[] = "r1bq1rk1/pp1pnppp/1b1p4/n2N2B1/2B1P3/5N2/P4PPP/R2QK2R b KQ - 1 12";
  board b(fen);

  setmasks(b);
  WPmoves(b);
  WNmoves(b);
  WRmoves(b);
  return 0;
}
