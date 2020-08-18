#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#define NOINFO 0xF
#define KRED  "\x1B[31m"
#define KNRM  "\x1B[0m"
#define KYEL "\x1B[33m"

typedef enum {None,Pawn,Rook,Knight,Bishop,Queen,King} piecetype;
typedef enum{White,Black} color;
typedef struct{
  piecetype p;
  color c;
} piece;

typedef struct{
  char file;
  char rank;
} square;

typedef struct{
  short fullmoves;
  square to;
  square from;
  piece promote;
  piece piece;
  char takes;
  char checks;
} move;

piecetype ptfromc(char c){
  switch(c){
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
  case '_':
    return None;
  default:
    fprintf(stderr, "Error, tried to read %c as piece\n",c);
    exit(1);
  }
}
char cfrompt(piecetype p){
  switch(p){
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
    fprintf(stderr, "Error, invalid piece enum, read %d\n",p);
    exit(1);
  }
}

color clfromc(char c){
  switch(c){
  case 'W':
    return White;
  case 'B':
    return Black;
  default:
    fprintf(stderr, "Error, invalid color char, read %c\n",c);
    exit(1);
  }
}

char ffromc(char c){
  if (c >= 'a' && c <= 'h')
    return c - 'a';
  else if (c == '_')
    return NOINFO;
  fprintf(stderr, "Error, not a valid file char, read %c\n",c);
  exit(1);
}


char rfromc(char c){
  if (c >= '1' && c <= '9')
    return c - '1';
  else if (c == '_')
    return NOINFO;
  fprintf(stderr, "Error, not a valid rank char, read %c\n",c);
  exit(1);
}


move readmove(){
  move m;
  m.fullmoves = 0;
  m.fullmoves += 100 * (getchar() - '0');
  m.fullmoves += 10 * (getchar() - '0');
  m.fullmoves += (getchar() - '0');

  m.piece.c = clfromc(getchar());
  m.piece.p = ptfromc(getchar());

  m.from.file = ffromc(getchar());
  m.from.rank = rfromc(getchar());

  m.to.file = ffromc(getchar());
  m.to.rank = rfromc(getchar());

  char c;
  switch (c = getchar()){
  case '_':
    m.takes = 0; break;
  case 'x':
    m.takes = 1; break;
  default:
    fprintf(stderr, "Error, invalid takes char, read %c\n",c);
    exit(1);
  }

  
  switch (c = getchar()){
  case '_':
    m.checks = 0; break;
  case '+':
  case '#':
    m.checks = 1; break;
  default:
    fprintf(stderr, "Error, invalid check char, read %c\n",c);
    exit(1);
  }

  m.promote.c = m.piece.c; /* promoting piece keeps color */
  m.promote.p = ptfromc(getchar());

  return m;
}


typedef struct{
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

boardstate newboard(int gameid){
  boardstate b;
  b.gameid = gameid;
  for(char i = 0; i < 8; i++){
    b.board[i][1] = (piece){Pawn, White};
    b.board[i][6] = (piece){Pawn, Black};
    for(char j = 2; j < 6; j++)
      b.board[i][j] = (piece){None,White};
  }

  b.board[0][0] = b.board[7][0] = (piece){Rook, White};
  b.board[1][0] = b.board[6][0] = (piece){Knight, White};
  b.board[2][0] = b.board[5][0] = (piece){Bishop, White};
  b.board[3][0] = (piece){Queen, White};
  b.board[4][0] = (piece){King, White};

  
  b.board[0][7] = b.board[7][7] = (piece){Rook, Black};
  b.board[1][7] = b.board[6][7] = (piece){Knight, Black};
  b.board[2][7] = b.board[5][7] = (piece){Bishop, Black};
  b.board[3][7] = (piece){Queen, Black};
  b.board[4][7] = (piece){King, Black};

  b.fullmoves = 1;
  b.halfmoveclock = 0;

  b.tomove = White;

  b.canep = 0;

  b.wkcastle = b.wqcastle = b.bkcastle = b.bqcastle = 1;
  return b;
}

void fprettyprintboard(FILE* stream, boardstate b){
  fprintf(stream, "%d. %s to play (Game %d),\n",b.fullmoves,b.tomove == White? "White": "Black",b.gameid);

  fprintf(stream,KYEL"╔═════════════════╗\n"KNRM);

  for (int i = 7; i >= 0; --i){

    fprintf(stream, KYEL"║"KNRM);

    for (int j = 0; j < 8; ++j){
      char c = 0;
      piece p = b.board[j][i];
      if (p.c == Black){
        fprintf(stream,KRED);
        c += 'a' - 'A';
      }

      c += cfrompt(p.p);
      if ( b.canep && j == b.epsquare.file && i == b.epsquare.rank)
        fprintf(stream,KYEL" ."KNRM);
      else
        fprintf(stream," %c"KNRM,c);
    }
    fprintf(stream,KYEL" ║  %c\n"KNRM,i + '1');
  }
  fprintf(stream, KYEL"╚═════════════════╝\n  a b c d e f g h"KNRM"\n");

}

void prettyprintboard(boardstate b){
  fprettyprintboard(stdout,b);
}

char onboard(square s){
  return (s.rank >= 0 && s.rank < 8 && s.file >= 0 && s.file < 8);
}

void fprintsq(FILE *stream, square s){
  fprintf(stream, "%c%c",s.file + 'a', s.rank + '1');
}

void printsq(square s){
  printf("%c%c",s.file + 'a', s.rank + '1');
}

color other(color c){
  return c == White ? Black : White;
}

typedef struct{
  square s[8];
  char len;
} squarelist;

piece at(boardstate *p_b,square s){
  return (*p_b).board[s.file][s.rank];
}

char samep(piece p1,piece p2){
  return (p1.p == p2.p) && (p1.c == p2.c);
}

squarelist pawncand(move m, boardstate b){
  squarelist candidate;
  candidate.len = 0;
  square from;

  
  from = (square){m.to.file, (m.piece.c? 1 : -1) + m.to.rank};

  if (samep(at(&b,from),m.piece)){
    candidate.s[candidate.len] = from;
    candidate.len++;
  }

  /* we need the previous square to be free, to double move */
  square free = from;
  from = (square){m.to.file, m.piece.c? 6 : 1};
  if (m.to.rank == 3 + m.piece.c && at(&b,free).p == None){
    candidate.s[candidate.len] = from;
    candidate.len++;
  }

  /* attacking moves */
  from = (square){m.to.file-1,(m.piece.c? 1 : -1) + m.to.rank};
  if (onboard(from) && samep(at(&b,from),m.piece)){
    char validattack = at(&b,m.to).p != None && at(&b,m.to).c == other(m.piece.c);
    char validep = b.canep && m.to.rank == b.epsquare.rank && m.to.file == b.epsquare.file;
    if(validattack || validep){
      candidate.s[candidate.len] = from;
      candidate.len++;
    }
  }
  
  from = (square){m.to.file+1,(m.piece.c? 1 : -1) + m.to.rank};

  if (onboard(from) && samep(at(&b,from),m.piece)){
    char validattack = at(&b,m.to).p != None && at(&b,m.to).c == other(m.piece.c);
    char validep = b.canep && m.to.rank == b.epsquare.rank && m.to.file == b.epsquare.file;
    if(validattack || validep){
      candidate.s[candidate.len] = from;
      candidate.len++;
    }
  }
  
  return candidate;
}

square addsq(square s1, square s2){
  return (square){s1.file + s2.file, s1.rank + s2.rank};
}

squarelist stepuntill(square step, move m, boardstate *p_b, squarelist *p_cand){
  square from = addsq(m.to,step);
  
  while (onboard(from) && at(p_b,from).p == None){
    from = addsq(from,step);
  }
  if (onboard(from) && samep(at(p_b,from),m.piece)){
    p_cand->s[p_cand->len] = from;
    p_cand->len++;
  }
  return (*p_cand);
}

squarelist bishopcand(move m, boardstate b){
  squarelist candidate;
  candidate.len = 0;

  square step;
  
  step = (square){-1,-1};
  stepuntill(step,m,&b,&candidate);
  step = (square){1,-1};
  stepuntill(step,m,&b,&candidate);
  step = (square){-1,1};
  stepuntill(step,m,&b,&candidate);
  step = (square){1,1};
  stepuntill(step,m,&b,&candidate);
  return candidate;
}

squarelist rookcand(move m, boardstate b){
  squarelist candidate;
  candidate.len = 0;

  square step;
  step = (square){0,-1};
  stepuntill(step,m,&b,&candidate);
  step = (square){0,1};
  stepuntill(step,m,&b,&candidate);
  step = (square){-1,0};
  stepuntill(step,m,&b,&candidate);
  step = (square){1,0};
  stepuntill(step,m,&b,&candidate);
  
  return candidate;
}

squarelist queencand(move m, boardstate b){
  squarelist candidate;
  candidate.len = 0;

  square step;
  
  step = (square){-1,-1};
  stepuntill(step,m,&b,&candidate);
  step = (square){1,-1};
  stepuntill(step,m,&b,&candidate);
  step = (square){-1,1};
  stepuntill(step,m,&b,&candidate);
  step = (square){1,1};
  stepuntill(step,m,&b,&candidate);
  step = (square){0,-1};
  stepuntill(step,m,&b,&candidate);
  step = (square){0,1};
  stepuntill(step,m,&b,&candidate);
  step = (square){-1,0};
  stepuntill(step,m,&b,&candidate);
  step = (square){1,0};
  stepuntill(step,m,&b,&candidate);
  return candidate;
}


squarelist onestep(square step, move m, boardstate *p_b, squarelist *p_cand){
  square from = addsq(m.to,step);
  
  if (onboard(from) && samep(at(p_b,from),m.piece)){
    p_cand->s[p_cand->len] = from;
    p_cand->len++;
  }
  return (*p_cand);
}

squarelist kingcand(move m, boardstate b){
  squarelist candidate;
  candidate.len = 0;

  square step;
  
  step = (square){-1,-1};
  onestep(step,m,&b,&candidate);
  step = (square){1,-1};
  onestep(step,m,&b,&candidate);
  step = (square){-1,1};
  onestep(step,m,&b,&candidate);
  step = (square){1,1};
  onestep(step,m,&b,&candidate);
  step = (square){0,-1};
  onestep(step,m,&b,&candidate);
  step = (square){0,1};
  onestep(step,m,&b,&candidate);
  step = (square){-1,0};
  onestep(step,m,&b,&candidate);
  step = (square){1,0};
  onestep(step,m,&b,&candidate);
  
  return candidate;
}
squarelist knightcand(move m, boardstate b){
  squarelist candidate;
  candidate.len = 0;

  square step;
  
  step = (square){-1,-2};
  onestep(step,m,&b,&candidate);
  step = (square){1,-2};
  onestep(step,m,&b,&candidate);
  step = (square){-1,2};
  onestep(step,m,&b,&candidate);
  step = (square){1,2};
  onestep(step,m,&b,&candidate);
  step = (square){-2,-1};
  onestep(step,m,&b,&candidate);
  step = (square){2,-1};
  onestep(step,m,&b,&candidate);
  step = (square){-2,1};
  onestep(step,m,&b,&candidate);
  step = (square){2,1};
  onestep(step,m,&b,&candidate);

  return candidate;
}

move* movecontext(move* p_m, boardstate b){
  squarelist candidate;
  
  move m = *p_m;
  
  switch(m.piece.p){
  case Pawn:
    candidate = pawncand(m,b); break;
  case Rook:
    candidate = rookcand(m,b); break;
  case Knight:
    candidate = knightcand(m,b); break;
  case Bishop:
    candidate = bishopcand(m,b); break;
  case Queen:
    candidate = queencand(m,b); break;
  case King:
    candidate = kingcand(m,b);
    if (candidate.len > 0 || p_m->from.file != 4) break;
    p_m->to.rank = p_m->from.rank = p_m->piece.c? 7 : 0;
    return p_m;
  default:
    fprintf(stderr, "Error, invalid piece type enum, or None piece given when illegal, read %d",m.piece.p);
    exit(1);
  }

  if (p_m->from.file != NOINFO){
    squarelist temp;
    temp.len = 0;
    for (int i = 0; i < candidate.len; ++i){
      if (candidate.s[i].file == p_m->from.file){
        temp.s[temp.len] = candidate.s[i];
        temp.len++;
      }
    }
    candidate = temp;
  }

  if (p_m->from.rank != NOINFO){
    squarelist temp;
    temp.len = 0;
    for (int i = 0; i < candidate.len; ++i){
      if (candidate.s[i].rank == p_m->from.rank){
        temp.s[temp.len] = candidate.s[i];
        temp.len++;
      }
    }
    candidate = temp;
  }
  
  if (candidate.len != 1){
    fprettyprintboard(stderr, b);
    fprintf(stderr, "Error, either no, or too many candidate positions\n");
    fprintf(stderr, "candidate origins : %d,  \t",candidate.len);
    for( int i = 0; i < candidate.len; ++i){
      fprintsq(stderr, candidate.s[i]);
      fprintf(stderr, " ");
    }
    fprintf(stderr, "\n%c going to ",cfrompt(p_m->piece.p));
    fprintsq(stderr, p_m->to);
    fprintf(stderr,"\n");
    exit(1);
  }
  p_m->from = candidate.s[0];
  return p_m;
}
  
boardstate* makemove(boardstate *p_b, move m){
  if (m.piece.p == King && m.from.file == 4){ /* possible castle? */
    if(m.to.file == 2){ /* queenside */
      p_b->board[3][m.from.rank] = p_b->board[0][m.from.rank];
      p_b->board[0][m.from.rank] = (piece){None,White};
    }
    if(m.to.file == 6){ /* kingside */
      p_b->board[5][m.from.rank] = p_b->board[7][m.from.rank];
      p_b->board[7][m.from.rank] = (piece){None,White};
    }
  }
  
  p_b->board[m.to.file][m.to.rank] = p_b->board[m.from.file][m.from.rank];
  p_b->board[m.from.file][m.from.rank] = (piece){None,White};

  if (m.promote.p != None){ /* promotion */
    p_b->board[m.to.file][m.to.rank] = m.promote;
    assert(m.to.rank == m.piece.c? 0 : 7);
  }
  /* if en passant, remove passing pawn */
  if (p_b->canep && m.piece.p == Pawn && m.to.rank == p_b->epsquare.rank && m.to.file == p_b->epsquare.file){
    p_b->board[m.to.file][m.piece.c? 3 : 4] = (piece){None,White};
  } 
  

  /* set en passant target square square */
  if (m.piece.p == Pawn && m.from.rank == (m.piece.c? 6 : 1) && m.to.rank == (m.piece.c? 4 : 3)){
    p_b->canep = 1;
    p_b->epsquare = (square){m.to.file, m.piece.c? 5 : 2};
  }else{
    p_b->canep = 0;
  }
    
  
  p_b->fullmoves = m.fullmoves;
  p_b->tomove = other(m.piece.c);
  if (p_b->tomove == White)
    p_b->fullmoves++;
  return p_b;
}

void playmove(boardstate *p_b){
  move m = readmove();
  movecontext(&m,*p_b);
  /*printsq(m.to);
  printf("<-");
  printsq(m.from);
  printf("\n");*/
  makemove(p_b,m);
}

void prettyendgame(char c){
  switch(c){
  case('W'):
    printf("White Wins!\n"); break;
  case('B'):
    printf("Black Wins!\n"); break;
  case('D'):
    printf("Draw!\n"); break;
  default:
    fprintf(stderr,"Error, bad char between moves\n");
    exit(1);
  }
}

int main(){
  move m;
  char c;
  int game = 0;
  do{
    boardstate b = newboard(game);
    prettyprintboard(b);
    printf("\n");
    do{
      playmove(&b);
      prettyprintboard(b);
    } while ((c = getchar()) == '\t');
    prettyendgame(c);
    game++;
    assert((c = getchar()) == '\n' || c == EOF);
  } while (c != EOF);
  printf("Whew...\n");
}
