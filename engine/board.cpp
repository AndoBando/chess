//
// Created by andy on 9/27/2020.
//
#define KRED  "\x1B[31m"
#define KNRM  "\x1B[0m"
#define KYEL "\x1B[33m"

#include "board.h"



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
      throw std::invalid_argument("Invalid piece char");
  }
}

char cfromp(const piece& p){
  constexpr char charcodes[] = "PRNBQKprnbqk";
  return charcodes[p];
}

void board::clearboard() {
  for (auto &bits : (*this)) {
    bits = 0;
  }
}

board::board() : array() {
}

U64 onehot(U8 r, U8 f){
  return (U64)1 << (8*r + f);
}

U64 onehot(square s){
  return (U64)1 << s;
}

board::board(const std::string &fenstring) {
  this->clearboard();

  char c;
  int charstraversed = 0;

  for (U8 r = 7; r != 0xFF ; --r) {
    for (U8 f = 0; f < 8; ++f) {
      c = fenstring[charstraversed++];
      if (c <= '8' && c >= '1') {
        f += c - '1';
        continue;
      }
      (*this)[pfromc(c)] |= onehot(r,f);
    }
    c = fenstring[charstraversed++];
    if ( c != ( (r != 0) ? '/' : ' ')) {
      throw std::invalid_argument("FEN rank not terminated, or longer than expected");
    }
  }
}

std::ostream& operator<<(std::ostream& os, const board& b){

  os << KYEL << "╔═════════════════╗\n" << KNRM;
  for (U8 r = 7; r != 0xFF; --r) {
    os << KYEL << "║" << KNRM;
    for (U8 f = 0; f < 8; ++f) {
      char c = ' ';
      forpieces( [&](piece p) {
        if (b[p] & onehot(r,f)) {
          c = cfromp(p);
          return;
        }
      });
      os << KNRM << " " << (c > 'a'? KRED:KNRM) << c;
    }
    os << KYEL << " ║ " << r + 1 <<  "\n" << KNRM;
  }
  os << KYEL"╚═════════════════╝\n  a b c d e f g h" << KNRM << "\n";
}



std::ostream &operator<<(std::ostream &os, const bbits &bb) {

  os << KYEL << "╔═════════════════╗\n" << KNRM;
  for (U8 r = 7; r != 0xFF; --r) {
    os << KYEL << "║" << KNRM;
    for (U8 f = 0; f < 8; ++f) {
      os << KRED << " " << (bb.bits & onehot(r,f)? "*":" ") << KNRM;
    }
    os << KYEL << " ║ " << r + 1 <<  "\n" << KNRM;
  }
  os << KYEL"╚═════════════════╝\n  a b c d e f g h" << KNRM << "\n";
}

U64 reflectH(const U64& bits_){
  auto bits = bits_;
  bits = (bits & 0xF0F0F0F0F0F0F0F0) >> 4 | (bits & 0x0F0F0F0F0F0F0F0F) << 4;
  bits = (bits & 0xCCCCCCCCCCCCCCCC) >> 2 | (bits & 0x3333333333333333) << 2;
  bits = (bits & 0xAAAAAAAAAAAAAAAA) >> 1 | (bits & 0x5555555555555555) << 1;
  return bits;
}
U64 reflectV(const U64& bits_){
  auto bits = bits_;
  bits = (bits & 0xFFFFFFFF00000000) >> 32 | (bits & 0x00000000FFFFFFFF) << 32;
  bits = (bits & 0xFFFF0000FFFF0000) >> 16 | (bits & 0x0000FFFF0000FFFF) << 16;
  bits = (bits & 0xFF00FF00FF00FF00) >> 8 | (bits & 0x00FF00FF00FF00FF) << 8;
  return bits;
}


