#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

#define TRUE 1
#define FALSE 0
#define NOINFO 0xF
#define WHITE 1
#define DRAW 2
#define BLACK 3
#define KRED  "\x1B[31m"
#define KNRM  "\x1B[0m"
#define KYEL "\x1B[33m"

/* acts like getchar, but skips sections in [brackets] */
char nextnobr() {
  char c;
  c = getchar();
  /* if we hit a open bracket, read until a close then try again */
  if (c == '[') {
    while ( (c = getchar()) != EOF && c != ']')
      ;
    c = nextnobr();
  }
  return c;
}

/* like getchar, but skips [brackets], and replaces repeated whitespace with
   one space  */
char nextchar() {
  char c = nextnobr();
  static char p = ' '; /* previous char */
  while (isspace(c) && isspace(p) && c != EOF) {
    c = nextnobr();
  }
  if (isspace(c))
    c = ' ';
  p = c;
  return c;
}

int readmovenum(int movenum) {
  char c;
  while ((c = nextchar()) != '.') {
    if (isdigit(c))
      movenum = 10 * movenum + c - '0';
    else {
      fprintf(stderr,
              "Error, invalid movenum char (digit), read %c (%x) a\n", c, c);
      exit(1);
    }
  }
  return movenum;
}

/* Piecetype is different from piece, as piece contains color info */
typedef enum {None, Pawn, Rook, Knight, Bishop, Queen, King} piecetype;

typedef enum {White = 0, Black = 1} color;
/* we'll make use of the, sometimes confusing, property that White is false and
   black is true */

typedef struct {
  piecetype pt : 3; /* not sure this does anything lol */
  color cl : 1;
} piece;

char isfile(char c) {
  return (c >= 'a' && c <= 'h');
}
char isrank(char c) {
  return (c >= '1' && c <= '8');
}

typedef struct {
  char file;
  char rank;
} square;



char onboard(square s) {
  return (s.rank >= 0 && s.rank < 8 && s.file >= 0 && s.file < 8);
}


color other(color c) {
  return c == White ? Black : White;
}

char samep(piece p1, piece p2) {
  return (p1.pt == p2.pt) && (p1.cl == p2.cl);
}

char samesq(square s1, square s2) {
  return (s1.file == s2.file) && (s1.rank == s2.rank);
}

const char* namefrompt(piecetype pt) {
  switch (pt) {
  case Pawn:
    return "pawn";
  case Rook:
    return "rook";
  case Knight:
    return "knight";
  case Bishop:
    return "bishop";
  case Queen:
    return "queen";
  case King:
    return "king";
  default:
    fprintf(stderr, "Error, invalid piece type enum,"
            "or None piece given when illegal, "
            "read %d", pt);
    exit(1);
  }
}

piecetype ptfromc(char c) {
  if (isdigit(c))
    return None;
  if (isfile(c))
    return Pawn;
  switch (c) {
  case 'P':
    return Pawn;
  case 'R':
    return Rook;
  case 'N':
    return Knight;
  case 'B':
    return Bishop;
  case 'Q':
    return Queen;
  case 'K':
    return King;
  case 'O':
  case '_':
    return None;
  default:
    fprintf(stderr, "Error, tried to read %c as piece\n", c);
    exit(1);
  }
}

char cfrompt(piecetype pt) {
  switch (pt) {
  case Pawn:
    return 'P';
  case Rook:
    return 'R';
  case Knight:
    return 'N';
  case Bishop:
    return 'B';
  case Queen:
    return 'Q';
  case King:
    return 'K';
  case None:
    return ' ';
  default:
    fprintf(stderr, "Error, invalid piece enum, read %d\n", pt);
    exit(1);
  }
}

typedef struct {
  short movenum;
  piece piece;
  piece promote;
  square to;
  square from;
  char takes;
  char check;
  char mate;
  char promotes;
  char castle; /* not sure we need */
  char wins;
  char over;
  char isnum;
} move;


char moveended(move* p_m, char c) {
  switch (c) { /* no breaks! */
  case '#':
    p_m->mate = TRUE;
  case '+':
    p_m->check = TRUE;
    c = nextchar(); /* we need to check the next char is really a ' ' */
  /* also, it okay to change c inside our function, */
  /* b/c we expect to end if we've made it here     */
  case ' ':
    if (c != ' ') /* is it really? */
      fprintf(stderr, "Error, move ended by + or #, but not followed by a "
              "space. Instead read %c (%x)\n", c, c);
    return TRUE;
  default:
    return FALSE;
    ;
  }
}

char promotes(move* p_m, char c) {
  if (c == '=') {
    p_m->promotes = TRUE;
    if ((p_m->promote.pt = ptfromc(nextchar())) != None)
      return TRUE;
    else {
      fprintf(stderr, "BAD 1st PROMOTION READ_MOVE @ char c = %c (%x)\n", c, c);
      exit(1);
    }
  }
  return FALSE;
}

move readmove(short fullmoves, color cl) {
  char c;
  move m;

  /* where we're moving to */
  m.to.file = NOINFO;
  m.to.rank = NOINFO;
  /* where we came from, given only to disambiguate */
  m.from.file = NOINFO;
  m.from.rank = NOINFO;

  m.takes = FALSE;
  m.check = FALSE;
  m.mate = FALSE;
  m.castle = FALSE;
  m.wins = FALSE;
  m.movenum = fullmoves;
  m.over = FALSE;
  m.isnum = FALSE;
  m.promotes = FALSE;
  m.promote.pt = None;
  m.promote.cl = cl;
  m.piece.cl = cl;

  c = nextchar();

  if (c == EOF) {
    m.over = TRUE;
    return m;
  }

  /* we switch on the first char to get the piece */

  m.piece.pt = ptfromc(c);

  if (m.piece.pt == None) {
    switch (c) {
    case 'O': /* castle */

      m.piece.pt = King; /* we consider castling to be the kings move */

      /* the next chars better be -O */
      if (!(nextchar() == '-' && nextchar() == 'O')) {
        fprintf(stderr, "Error, invalid char while reading castle,"
                "read %c (%x)\n", c, c);
        exit(1);
      }

      m.castle = TRUE;
      m.from.file = 'e' - 'a'; /* king must have been on the `e` file */

      if ((c = nextchar()) != '-') { /* King side? */
        m.to.file = 'g' - 'a';
      } else if (c == '-' && nextchar() == 'O') {
        c = nextchar();
        m.to.file = 'c' - 'a';
      } else {
        fprintf(stderr, "Error, invalid char while reading queenside castle,"
                "read %c (%x)\n", c, c);
        exit(1);
      }
      break;

    case '1':
      /* we begin with 1 in four cases, "1-0", "1/2-1/2", "1." or "17." */
      if ((c = nextchar()) == '-'
          && nextchar() == '0'
          && nextchar() == ' ') {
        m.wins = WHITE;
        return m;
      } else if (c == '/'
                 && nextchar() == '2'
                 && nextchar() == '-'
                 && nextchar() == '1'
                 && nextchar() == '/'
                 && nextchar() == '2'
                 && nextchar() == ' ') {
        m.wins = DRAW;
      } else if (c == '.') {
        m.isnum = TRUE;
        m.movenum = 1;
      } else if (isdigit(c)) {
        m.isnum = TRUE;
        m.movenum = readmovenum(10 + c - '0');
      } else {
        fprintf(stderr, "Error, invalid move starting with '1'"
                "read  %c (%x)\n", c, c);
        exit(1);
      }
      return m;
    case '0':
      if ((c = nextchar()) == '-'
          && nextchar() == '1'
          && nextchar() == ' ') {
        m.wins = BLACK;
        return m;
      }
      fprintf(stderr, "Error, invalid move starting with '0'"
              "read  %c (%x)\n", c, c);
      exit(1);
    default:
      if (isdigit(c)) {
        m.isnum = TRUE;
        m.movenum = readmovenum(c - '0');
        return m;
      } else {
        fprintf(stderr, "Error, move read begining with %c (%x)\n", c, c);
        exit(1);
      }
    }
  }

  if (moveended(&m, c))
    return m;

  if (m.piece.pt == Pawn)
    m.to.file = c; /* we need to remember this file */
  else
    c = nextchar();

  /* if have an x immediately after the first char, we have no disambiguation
  info this cannot apply to a pawn */

  if (c == 'x') {
    m.takes = TRUE;
    c = nextchar();
  }

  /* most of the time we expect a file here, if we get a rank it is
     disambiguating */

  if (isfile(c)) {
    m.to.file = c - 'a'; /* this might be disambiguating, but we assume not for now.
                            so we put it in `to`*/
  } else if (isrank(c)) {
    m.from.rank = c - '1';  /* this is definitely a disambiguating rank,
                               because we can't have seen a file yet*/

    /* if we've already taken, no disambiuating info can be given */
    assert(!m.takes);
  } else {
    fprintf(stderr, "Error, expected file or rank char, read %c (%x)\n", c, c);
    exit(1);
  }

  c = nextchar();

  /* check for takes */
  if (c == 'x') {
    m.takes = TRUE;
    c = nextchar();
  }

  /* here we expect a rank most of the time */
  if (isrank(c)) {
    m.to.rank = c - '1';
  } else if (isfile(c)) {
    /* if we get a file, if the previous file was set, it
       it was disambiguating, otherwise the previous rank was set */
    if (m.to.file != NOINFO)
      m.from.file = m.to.file;

    m.to.file = c - 'a';
  } else {
    fprintf(stderr, "Error, expected file or rank char, read %c (%x)\n", c, c);
    exit(1);
  }

  c = nextchar();

  if (promotes(&m, c))
    c = nextchar();

  if (moveended(&m, c))
    return m;

  if (c == 'x') {
    m.takes = TRUE;
    c = nextchar();
  }

  if (isrank(c)) {
    m.to.rank = c - '1'; /* if we make it here we should be done. */
  } else if (isfile(c)) {
    m.from.file = m.to.file;
    m.from.rank = m.to.rank;
    m.to.file = c - 'a';
    m.to.rank = NOINFO;
  } else {
    fprintf(stderr, "Error, expected file or rank char, read %c (%x)\n", c, c);
    exit(1);
  }

  c = nextchar();

  if (promotes(&m, c))
    c = nextchar();

  if (moveended(&m, c))
    return m;

  /* if we're not done yet, the last char must be a rank */
  if (isrank(c)) {
    m.to.rank = c - '1';
  } else {
    fprintf(stderr, "Error, expected file or rank char, read %c (%x)\n", c, c);
    exit(1);
  }

  c = nextchar();

  if (promotes(&m, c))
    c = nextchar();

  if (moveended(&m, c))
    return m;

  /* If we havent ended here, thats bad */
  fprintf(stderr, "Error, move not ended when expected, read %c (%x)\n", c, c);
  exit(1);

  /* Whew... */
}

void fprintmovefixedsize(FILE* stream, move m) {

  fprintf(stream, "%c", cfrompt(m.piece.pt));

  if (m.from.file == NOINFO)
    fprintf(stream, "_");
  else
    fprintf(stream, "%c", m.from.file + 'a');

  if (m.from.rank == NOINFO)
    fprintf(stream, "_");
  else
    fprintf(stream, "%c", m.from.rank + '1');

  if (m.to.file == NOINFO)
    fprintf(stream, "_");
  else
    fprintf(stream, "%c", m.to.file + 'a');

  if (m.to.rank == NOINFO)
    fprintf(stream, "_");
  else
    fprintf(stream, "%c", m.to.rank + '1');

  if (m.takes == FALSE)
    fprintf(stream, "_");
  else
    fprintf(stream, "x");

  if (m.mate)
    fprintf(stream, "#");
  else if (m.check)
    fprintf(stream, "+");
  else
    fprintf(stream, "_");

  if (m.promotes)
    fprintf(stream, "%c", cfrompt(m.promote.pt));
  else
    fprintf(stream, "_");
}

void fprintmove(FILE* stream, move m) {

  if (m.piece.pt != Pawn)
    fprintf(stream, "%c", cfrompt(m.piece.pt));

  if (m.from.file != NOINFO)
    fprintf(stream, "%c", m.from.file + 'a');

  if (m.from.rank != NOINFO)
    fprintf(stream, "%c", m.from.rank + '1');

  if (m.takes)
    fprintf(stream, "x");

  if (m.to.file != NOINFO)
    fprintf(stream, "%c", m.to.file + 'a');

  if (m.to.rank != NOINFO)
    fprintf(stream, "%c", m.to.rank + '1');

  if (m.promote.pt != None)
    fprintf(stream, "=%c", cfrompt(m.promote.pt));

  if (m.mate)
    fprintf(stream, "#");
  else if (m.check)
    fprintf(stream, "+");
}


typedef struct {
  piece board[8][8];
  short fullmoves;
  int gameid;
  short halfmoveclock;
  square epsquare;
  color tomove;
  char canep;
  char wkcastle;
  char wqcastle;
  char bkcastle;
  char bqcastle;
} boardstate;

piece* at(boardstate *p_b, square s) {
  return &(p_b->board[s.file][s.rank]);
}

boardstate newboard(int gameid) {
  boardstate b;
  b.gameid = gameid;
  for (char i = 0; i < 8; i++) {
    b.board[i][1] = (piece) {Pawn, White};
    b.board[i][6] = (piece) {Pawn, Black};
    for (char j = 2; j < 6; j++)
      b.board[i][j] = (piece) {None, White};
  }

  b.board[0][0] = b.board[7][0] = (piece) {Rook, White};
  b.board[1][0] = b.board[6][0] = (piece) {Knight, White};
  b.board[2][0] = b.board[5][0] = (piece) {Bishop, White};
  b.board[3][0] = (piece) {Queen, White};
  b.board[4][0] = (piece) {King, White};


  b.board[0][7] = b.board[7][7] = (piece) {Rook, Black};
  b.board[1][7] = b.board[6][7] = (piece) {Knight, Black};
  b.board[2][7] = b.board[5][7] = (piece) {Bishop, Black};
  b.board[3][7] = (piece) {Queen, Black};
  b.board[4][7] = (piece) {King, Black};

  b.fullmoves = 1;
  b.halfmoveclock = 0;

  b.tomove = White;

  b.canep = 0;

  b.wkcastle = b.wqcastle = b.bkcastle = b.bqcastle = 1;
  return b;
}

void fprettyprintboard(FILE* stream, boardstate* p_b) {
  fprintf(stream, "%d. %s to play (Game %d),\n",
          p_b->fullmoves,
          p_b->tomove == White ? "White" : "Black",
          p_b->gameid);

  fprintf(stream, KYEL"╔═════════════════╗\n"KNRM);

  for (int i = 7; i >= 0; --i) {

    fprintf(stream, KYEL"║"KNRM);

    for (int j = 0; j < 8; ++j) {
      char c = 0;
      piece p = p_b->board[j][i];
      if (p.cl == Black) {
        fprintf(stream, KRED);
        c += 'a' - 'A';
      }

      c += cfrompt(p.pt);
      if ( p_b->canep && j == p_b->epsquare.file && i == p_b->epsquare.rank)
        fprintf(stream, KYEL" ."KNRM);
      else
        fprintf(stream, " %c"KNRM, c);
    }
    fprintf(stream, KYEL" ║  %c\n"KNRM, i + '1');
  }
  fprintf(stream, KYEL"╚═════════════════╝\n  a b c d e f g h"KNRM"\n");

}

void prettyprintboard(boardstate* p_b) {
  fprettyprintboard(stdout, p_b);
}

void fprintsq(FILE *stream, square s) {
  fprintf(stream, "%c%c", s.file + 'a', s.rank + '1');
}

void printsq(square s) {
  printf("%c%c", s.file + 'a', s.rank + '1');
}

void fprintboard(FILE* stream, boardstate b) {

  for (int i = 7; i >= 0; --i) {
    char spaces = 0;
    for (int j = 0; j < 8; ++j) {
      char c = 0;
      piece p = b.board[j][i];
      if (p.cl == Black) {
        c += 'a' - 'A';
      }
      if (p.pt != None) {
        if (spaces != 0) {
          fprintf(stream, "%d", spaces);
        }
        spaces = 0;
        c += cfrompt(p.pt);
        fprintf(stream, "%c", c);
      } else {
        spaces++;
      }
    }
    if (spaces != 0)
      fprintf(stream, "%d", spaces);
    if (i != 0)
      fprintf(stream, "/");
  }
  fprintf(stream, " %c ", b.tomove ? 'b' : 'w');
  if (b.wkcastle)
    fprintf(stream, "K");
  if (b.wqcastle)
    fprintf(stream, "Q");
  if (b.bkcastle)
    fprintf(stream, "k");
  if (b.bqcastle)
    fprintf(stream, "q");
  if (!(b.wkcastle || b.wqcastle || b.bkcastle || b.bqcastle))
    fprintf(stream, "-");
  fprintf(stream, " ");
  if (b.canep)
    fprintsq(stream, b.epsquare);
  else
    fprintf(stream, "-");
  fprintf(stream, " %d %d\n", b.halfmoveclock, b.fullmoves);
}

void printboard(boardstate b) {
  fprintboard(stdout, b);
}


typedef struct {
  square s[8];
  char len;
} squarelist;


squarelist pawncand(move m, boardstate* p_b) {
  squarelist candidate;
  candidate.len = 0;
  square from;


  from = (square) {m.to.file, (m.piece.cl ? 1 : -1) + m.to.rank};

  if (samep(*at(p_b, from), m.piece) && at(p_b, m.to)->pt == None) {
    candidate.s[candidate.len] = from;
    candidate.len++;
  }

  /* we need the previous square to be free, to double move */
  square free = from;
  from = (square) {m.to.file, m.piece.cl ? 6 : 1};

  if (m.to.rank == 3 + m.piece.cl &&
      at(p_b, free)->pt == None &&
      at(p_b, m.to)->pt == None) {
    candidate.s[candidate.len] = from;
    candidate.len++;
  }

  /* attacking moves */
  from = (square) {m.to.file - 1, (m.piece.cl ? 1 : -1) + m.to.rank};

  if (onboard(from) && samep(*at(p_b, from), m.piece)) {

    char validattack = at(p_b, m.to)->pt != None
                       && at(p_b, m.to)->cl == other(m.piece.cl);

    char validep = p_b->canep && samesq(m.to, p_b->epsquare);

    if (validattack || validep) {
      candidate.s[candidate.len] = from;
      candidate.len++;
    }
  }

  from = (square) {m.to.file + 1, (m.piece.cl ? 1 : -1) + m.to.rank};

  if (onboard(from) && samep(*at(p_b, from), m.piece)) {

    char validattack = at(p_b, m.to)->pt != None
                       && at(p_b, m.to)->cl == other(m.piece.cl);

    char validep = p_b->canep && samesq(m.to, p_b->epsquare);

    if (validattack || validep) {
      candidate.s[candidate.len] = from;
      candidate.len++;
    }
  }

  return candidate;
}

square addsq(square s1, square s2) {
  return (square) {s1.file + s2.file, s1.rank + s2.rank};
}

squarelist stepuntill(square step, move m, boardstate *p_b, squarelist *p_cand) {
  square from = addsq(m.to, step);

  while (onboard(from) && at(p_b, from)->pt == None) {
    from = addsq(from, step);
  }

  if (onboard(from) && samep(*at(p_b, from), m.piece)) {
    p_cand->s[p_cand->len] = from;
    p_cand->len++;
  }
  return (*p_cand);
}

squarelist bishopcand(move m, boardstate* p_b) {
  squarelist candidate;
  candidate.len = 0;

  square step;

  step = (square) { -1, -1};
  stepuntill(step, m, p_b, &candidate);
  step = (square) {1, -1};
  stepuntill(step, m, p_b, &candidate);
  step = (square) { -1, 1};
  stepuntill(step, m, p_b, &candidate);
  step = (square) {1, 1};
  stepuntill(step, m, p_b, &candidate);
  return candidate;
}

squarelist rookcand(move m, boardstate* p_b) {
  squarelist candidate;
  candidate.len = 0;

  square step;
  step = (square) {0, -1};
  stepuntill(step, m, p_b, &candidate);
  step = (square) {0, 1};
  stepuntill(step, m, p_b, &candidate);
  step = (square) { -1, 0};
  stepuntill(step, m, p_b, &candidate);
  step = (square) {1, 0};
  stepuntill(step, m, p_b, &candidate);

  return candidate;
}

squarelist queencand(move m, boardstate* p_b) {
  squarelist candidate;
  candidate.len = 0;

  square step;

  step = (square) { -1, -1};
  stepuntill(step, m, p_b, &candidate);
  step = (square) {1, -1};
  stepuntill(step, m, p_b, &candidate);
  step = (square) { -1, 1};
  stepuntill(step, m, p_b, &candidate);
  step = (square) {1, 1};
  stepuntill(step, m, p_b, &candidate);
  step = (square) {0, -1};
  stepuntill(step, m, p_b, &candidate);
  step = (square) {0, 1};
  stepuntill(step, m, p_b, &candidate);
  step = (square) { -1, 0};
  stepuntill(step, m, p_b, &candidate);
  step = (square) {1, 0};
  stepuntill(step, m, p_b, &candidate);
  return candidate;
}


squarelist onestep(square step, move m, boardstate *p_b, squarelist *p_cand) {
  square from = addsq(m.to, step);

  if (onboard(from) && samep(*at(p_b, from), m.piece)) {
    p_cand->s[p_cand->len] = from;
    p_cand->len++;
  }
  return (*p_cand);
}

squarelist kingcand(move m, boardstate* p_b) {
  squarelist candidate;
  candidate.len = 0;

  square step;

  step = (square) { -1, -1};
  onestep(step, m, p_b, &candidate);
  step = (square) {1, -1};
  onestep(step, m, p_b, &candidate);
  step = (square) { -1, 1};
  onestep(step, m, p_b, &candidate);
  step = (square) {1, 1};
  onestep(step, m, p_b, &candidate);
  step = (square) {0, -1};
  onestep(step, m, p_b, &candidate);
  step = (square) {0, 1};
  onestep(step, m, p_b, &candidate);
  step = (square) { -1, 0};
  onestep(step, m, p_b, &candidate);
  step = (square) {1, 0};
  onestep(step, m, p_b, &candidate);

  return candidate;
}
squarelist knightcand(move m, boardstate* p_b) {
  squarelist candidate;
  candidate.len = 0;

  square step;

  step = (square) { -1, -2};
  onestep(step, m, p_b, &candidate);
  step = (square) {1, -2};
  onestep(step, m, p_b, &candidate);
  step = (square) { -1, 2};
  onestep(step, m, p_b, &candidate);
  step = (square) {1, 2};
  onestep(step, m, p_b, &candidate);
  step = (square) { -2, -1};
  onestep(step, m, p_b, &candidate);
  step = (square) {2, -1};
  onestep(step, m, p_b, &candidate);
  step = (square) { -2, 1};
  onestep(step, m, p_b, &candidate);
  step = (square) {2, 1};
  onestep(step, m, p_b, &candidate);

  return candidate;
}

squarelist candidateorigins(move m, boardstate* p_b) {
  squarelist candidate;

  switch (m.piece.pt) {
  case Pawn:
    candidate = pawncand(m, p_b); break;
  case Rook:
    candidate = rookcand(m, p_b); break;
  case Knight:
    candidate = knightcand(m, p_b); break;
  case Bishop:
    candidate = bishopcand(m, p_b); break;
  case Queen:
    candidate = queencand(m, p_b); break;
  case King:
    candidate = kingcand(m, p_b);
    if (candidate.len > 0 || m.from.file != 4 || (m.to.file | 4) != 6 ) break;
    candidate.s[0].rank = m.piece.cl ? 7 : 0;
    candidate.s[0].file = m.from.file;
    candidate.len++;
    return candidate;
  default:
    fprintf(stderr, "Error, invalid piece type enum, or None piece given when "
            "illegal, read %d", m.piece.pt);
    exit(1);
  }

  /* do we have disamiguation info on file? */
  if (m.from.file != NOINFO) {
    squarelist temp;
    temp.len = 0;
    for (int i = 0; i < candidate.len; ++i) {
      if (candidate.s[i].file == m.from.file) {
        temp.s[temp.len] = candidate.s[i];
        temp.len++;
      }
    }
    candidate = temp;
  }
  /* Do we have disambiguation info on rank?*/
  if (m.from.rank != NOINFO) {
    squarelist temp;
    temp.len = 0;
    for (int i = 0; i < candidate.len; ++i) {
      if (candidate.s[i].rank == m.from.rank) {
        temp.s[temp.len] = candidate.s[i];
        temp.len++;
      }
    }
    candidate = temp;
  }

  return candidate;
}

square findapiece(piece p, boardstate* p_b) {
  for (int i = 0; i < 8; ++i)
    for (int j = 0; j < 8; ++j)
      if (samep(p_b->board[i][j], p))
        return (square) {i, j};
  return (square){NOINFO,NOINFO}
}

boardstate* makemove(move m, boardstate* p_b);

char incheck(boardstate b, color cl) {
  square kingsq = findapiece((piece) {King, cl}, &b);

  move tempattack;
  tempattack.to = kingsq;
  tempattack.from = (square) {NOINFO, NOINFO};
  tempattack.piece.cl = other(cl);

  for (piecetype pt = Pawn; pt <= King; pt++) {
    tempattack.piece.pt = pt;
    squarelist tempcand = candidateorigins(tempattack, &b);
    if (tempcand.len > 0) return 1;
  }
  return 0;
}

move* movecontext(move* p_m, boardstate* p_b) {
  squarelist candidate = candidateorigins(*p_m, p_b);

  /* to clear ambiguities b/c of check */
  if (candidate.len > 1) {
    squarelist tempcand;
    tempcand.len = 0;
    for (int i = 0; i < candidate.len; ++i) {

      boardstate tempb = *p_b;
      move tempm = (*p_m);
      tempm.from = candidate.s[i];
      makemove(tempm, &tempb);

      if (!incheck(tempb, p_m->piece.cl)) {
        tempcand.s[tempcand.len] = candidate.s[i];
        tempcand.len++;
      }
    }
    candidate = tempcand;
  }

  if (candidate.len != 1) {
    fprettyprintboard(stderr, p_b);
    fprintf(stderr, "Error, either no, or too many candidate positions\n");
    fprintf(stderr, "candidate origins : %d,  \t", candidate.len);
    for ( int i = 0; i < candidate.len; ++i) {
      fprintsq(stderr, candidate.s[i]);
      fprintf(stderr, " ");
    }
    fprintf(stderr, "\n%c going to ", cfrompt(p_m->piece.pt));
    fprintsq(stderr, p_m->to);
    fprintf(stderr, "\n");
    exit(1);
  }
  p_m->from = candidate.s[0];
  return p_m;
}

boardstate* makemove(move m, boardstate *p_b) {
  char emptakes = 0; /* Without looking, do we take? */
  p_b->halfmoveclock++;

  if (m.piece.pt == King && m.from.file == 4) { /* possible castle? */
    if (m.to.file == 2) { /* queenside */
      m.to.rank = m.from.rank;
      p_b->board[3][m.from.rank] = p_b->board[0][m.from.rank];
      p_b->board[0][m.from.rank] = (piece) {None, White};
    }
    if (m.to.file == 6) { /* kingside */
      m.to.rank = m.from.rank;
      p_b->board[5][m.from.rank] = p_b->board[7][m.from.rank];
      p_b->board[7][m.from.rank] = (piece) {None, White};
    }
  }
  if (at(p_b, m.to)->pt != None)
    emptakes = 1;

  /* if king moves, lose castling rights */
  if (m.piece.pt == King) {
    if (m.piece.cl == White)
      p_b->wkcastle = p_b->wqcastle = 0;
    else if (m.piece.cl == Black)
      p_b->bkcastle = p_b->bqcastle = 0;
  }

  /* if rook moves, lose that castling right */
  if (m.piece.pt == Rook) {
    if (m.from.rank == 0) {
      if (m.from.file == 0)
        p_b->wqcastle = 0;
      else if (m.from.file == 7)
        p_b->wkcastle = 0;
    } else if (m.from.rank == 7) {
      if (m.from.file == 0)
        p_b->bqcastle = 0;
      else if (m.from.file == 7)
        p_b->bkcastle = 0;
    }
  }

  *at(p_b, m.to) = *at(p_b, m.from);

  *at(p_b, m.from) = (piece) {None, White};

  if (m.promote.pt != None) { /* promotion */
    *at(p_b, m.to) = m.promote;
    assert(m.to.rank == m.piece.cl ? 0 : 7);
  }

  /* if en passant, remove passing pawn */
  if (p_b->canep && m.piece.pt == Pawn && samesq(m.to, p_b->epsquare)) {
    p_b->board[m.to.file][m.piece.cl ? 3 : 4] = (piece) {None, White};
    emptakes = 1;
  }

  /* if the notated takes doesn't corespond with our mesured take */
  if (m.takes != emptakes) {
    fprintf(stderr, "Notated 'takes' does not corespond to takes on board \n");
    fprintf(stderr, "Notated:%d, Measured:%d", m.takes, emptakes);
    fprettyprintboard(stderr, p_b);
    exit(1);
  }
  if (m.takes)
    p_b->halfmoveclock = 0;

  if (m.piece.pt == Pawn)
    p_b->halfmoveclock = 0;

  /* if check is notated, check that it really occurs */
  if (m.check && !incheck(*p_b, other(m.piece.cl))) {
    fprintf(stderr, "Notated 'checks' does not corespond to check on board \n");
    fprintf(stderr, "Notated:1, Measured:0\n");
    fprettyprintboard(stderr, p_b);
    fprintmove(stderr,m);
    exit(1);
  }


  /* set en passant target square */
  if (m.piece.pt == Pawn && m.from.rank == (m.piece.cl ? 6 : 1)
      && m.to.rank == (m.piece.cl ? 4 : 3)) {
    p_b->canep = 1;
    p_b->epsquare = (square) {m.to.file, m.piece.cl ? 5 : 2};
  } else {
    p_b->canep = 0;
  }


  p_b->fullmoves = m.movenum;
  p_b->tomove = other(m.piece.cl);
  if (p_b->tomove == White)
    p_b->fullmoves++;
  return p_b;
}

char fprettyendgame(char wins) {
  switch (wins) {
  case (WHITE):
    printf("White Wins!\n"); return 1;
  case (BLACK):
    printf("Black Wins!\n"); return 1;
  case (DRAW):
    printf("Draw!\n"); return 1;
  default:
    return 0;
  }
}

char endgame(char wins) {
  switch (wins) {
  case (WHITE):
    printf("W\n"); return 1;
  case (BLACK):
    printf("B\n"); return 1;
  case (DRAW):
    printf("D\n"); return 1;
  default:
    return 0;
  }
}


int main() {

  int plys = 0;
  int movenum;
  move m;
  int games = 0;
  m.over = FALSE;
  char c;
  do {
    printf("Game %d\n", games);
    games++;
    boardstate b = newboard(games);
    plys = 0;
    do {
      m = readmove(movenum, plys % 2);

      if (m.isnum){
        movenum = m.movenum;
        m = readmove(movenum, plys % 2);
      }

      if (m.over) break;
      if (endgame(m.wins)) break;

      // fprintmove(stderr, m);
      // fprintf(stderr, "\n");
      movecontext(&m, &b);
      makemove(m, &b);
      printboard(b);

    } while (plys++ < 2000 && m.wins == FALSE);
    assert((c = getchar()) == '\n' || c == EOF );
  } while (!m.over);
}