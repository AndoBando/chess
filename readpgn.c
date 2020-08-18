#include <stdio.h>
#include <ctype.h>

#define TRUE 1
#define FALSE 0
#define NOT_YET -1
#define WHITE 1
#define DRAW 2
#define BLACK 3



/* acts like getchar, but skips sections in [brackets] */
char next_no_br(){
  char c;
  c = getchar();
  /* if we hit a open bracket, read until a close then try again */
  if (c == '['){
    while ( (c = getchar()) != EOF && c != ']')
      ;
    c = next_no_br();
  }
  return c;
}

char next_char(){
  char c = next_no_br();
  static char p = ' '; /* previous char */
  while (isspace(c) && isspace(p) && c != EOF){
      c = next_no_br();
  }
  if (isspace(c))
      c = ' ';
  p = c;
  return c;
}

int read_move_num(int move_num){
  char c;

  while( (c = next_char()) != '.'){
    if ( c >= '0' && c <= '9')
      move_num = 10*move_num + c - '0';
    else
      printf("BAD MOVE_NUM READING @ char %x--%c\n",c,c);
  }
  return move_num;
}

typedef enum {None,Pawn,Rook,Knight,Bishop,Queen,King} piece;

const char* piece_name(piece p){
  switch (p){
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
      return "ERROR, NO SUCH PIECE";
  }
}

typedef struct {
  char file;
  char rank;
} square;

typedef struct {
  piece p;
  piece promotep;
  square to;
  square d;
  char takes;
  char check;
  char mate;
  char promotes;
  char castle;
  char wins;
  int move_num;
  char over;
} move;

char is_file(char c){
    return (c >= 'a' && c <= 'h');
}
char is_rank(char c){
    return (c >= '1' && c <= '8');
}

piece p_from_c(char c){
  switch (c){
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
    default:
      return None;
  }
}

move read_move(){
  char c;
  move m;

  /* where we're moving to */
  m.to.file = NOT_YET;
  m.to.rank = NOT_YET;
  /* where we came from, given only to disambiguate */
  m.d.file = NOT_YET;
  m.d.rank = NOT_YET;

  m.takes = FALSE;
  m.check = FALSE;
  m.mate = FALSE;
  m.castle = FALSE;
  m.wins = FALSE;
  m.move_num = 0;
  m.over = FALSE;

  m.promotes = FALSE;
  m.promotep = None;
  
  c = next_char();

  if(c == EOF){
    m.over = TRUE;
    return m;
  }

  /* we switch on the first char to get the piece */

  m.p = p_from_c(c);
  if(m.p == None) {
    switch (c) {
      case 'O':
        m.p = King;
        if (next_char() == '-' && next_char() == 'O') {
          m.castle = TRUE;
          m.d.file = 'e' - 'a'; /* king must have been on the `e` file */
          if ((c = next_char()) != '-') { /* king side */
            m.to.file = 'g' - 'a';
          } else if (c == '-' && next_char() == 'O') {
            c = next_char();
            m.to.file = 'c' - 'a';
          } else
            printf("BAD QUEENSIDE CASTLE IN READ_MOVE @ char c = %c (%x)\n", c, c);
        } else
          printf("BAD CASTLE IN READ_MOVE @ char c = %c (%x)\n", c, c);
        break;

      case '1':
        if ((c = next_char()) == '-'
            && next_char() == '0'
            && next_char() == ' ') {
          m.wins = WHITE;
          return m;
        } else if (c == '/'
                   && next_char() == '2'
                   && next_char() == '-'
                   && next_char() == '1'
                   && next_char() == '/'
                   && next_char() == '2'
                   && next_char() == ' ') {
          m.wins = DRAW;
        } else if (c == '.')
          m.move_num = 1;
        else if (c >= '0' && c <= '9')
          m.move_num = read_move_num(10 + c - '0');
        else
          printf("BAD WIN (1) IN READ_MOVE @ char c = %c (%x)\n", c, c);
        return m;
      case '0':
        if ((c = next_char()) == '-'
            && next_char() == '1'
            && next_char() == ' ') {
          m.wins = BLACK;
          return m;
        }
        printf("BAD WIN (1) IN READ_MOVE @ char c = %c (%x)\n", c, c);
        return m;
      default:
        if (is_file(c)) {
          m.p = Pawn;
          m.to.file = c; /* we need to remember this file */
        } else if (c >= '0' && c <= '9') {
          m.move_num = read_move_num(c - '0');
          return m;
        } else {
          printf("BAD READ_MOVE for piece char @ c = %c (%x)\n", c, c);
          return m;
        }
    }
  }

  switch (c) { /* no breaks! */
    case '#':
      m.mate = TRUE;
    case '+':
      m.check = TRUE;
      c = next_char(); /* we need to check the next char is really a ' ' */
    case ' ':
      if (c != ' ') /* is it really? */
        printf("BAD CASTLE ENDING @ char c = %c (%x)\n", c, c);
      return m;
    default:
      ;
  }

  if (m.p != Pawn)
    c = next_char();



  /* if have an x immediately after the first char, we have no disambiguation info
     this cannot apply to a pawn*/

  if (c == 'x'){
    m.takes = TRUE;
    c = next_char();
  }

  /* most of the time we expect a file here, if we get a rank it is disambiguating */

  if(is_file(c))
    m.to.file = c - 'a'; /* this might be disambiguating, but we assume not for now.
                          so we put it in `to`*/
  else if (is_rank(c))
    m.d.rank = c - '1';  /* this is definitely a disambiguating rank, because we can't
                            have seen a file yet*/
  else
    printf("BAD 1st RANK-FILE CHAR IN READ_MOVE @ char c = %c (%x)\n",c,c);


  c = next_char();

  /* check for takes */
  if (c == 'x'){
   m.takes = TRUE;
   c = next_char();
  }

  /* here we expect a rank most of the time */
  if(is_rank(c)){
    m.to.rank = c - '1';
  } else if (is_file(c)){ /* if we get a file, if the previous file was set, it
                                         it was disambiguating */
    if (m.to.file != NOT_YET){
      m.d.file = m.to.file;
      m.to.file = c - 'a';
    }
  } else
    printf("BAD 2nd RANK-FILE CHAR IN READ_MOVE @ char c = %c (%x)\n",c,c);

  c = next_char();

  if (c == '=') {
    m.promotes = TRUE;
    if ((m.promotep = p_from_c(next_char())) != None)
      c = next_char();
    else
      printf("BAD 1st PROMOTION READ_MOVE @ char c = %c (%x)\n", c, c);
  }


  switch (c){ /* no breaks! */
    case '#':
      m.mate = TRUE;
    case '+':
      m.check = TRUE;
      c = next_char(); /* we need to check the next char is really a ' ' */
    case ' ':
      if (c != ' ') /* is it really? */
        printf("BAD 1st END CHAR IN READ_MOVE @ char c = %c (%x)\n",c,c);
      return m;
    case 'x':
      m.takes = TRUE;
      c = next_char();
      break;
  }

  if (is_rank(c)){
    m.to.rank = c - '1'; /* if we make it here we should be done. */
  } else if (is_file(c)){
    m.d.file = m.to.file;
    m.d.rank = m.to.rank;
    m.to.file = c - 'a';
    m.to.rank = NOT_YET;
   } else
    printf("BAD 3rd RANK-FILE CHAR IN READ_MOVE @ char c = %c (%x)\n",c,c);


  c = next_char();

  if (c == '=') {
    m.promotes = TRUE;
    if ((m.promotep = p_from_c(next_char())) != None)
      c = next_char();
    else
      printf("BAD 2nd PROMOTION READ_MOVE @ char c = %c (%x)\n", c, c);
  }

  switch (c){ /* no breaks! */
    case '#':
      m.mate = TRUE;
    case '+':
      m.check = TRUE;
      c = next_char(); /* we need to check the next char is really a ' ' */
    case ' ':
      if (c != ' ') /* is it really? */
        printf("BAD 2nd END CHAR IN READ_MOVE @ char c = %c (%x)\n",c,c);
      return m;
   }

  /* if we're not done yet, the last char must be a rank */
  if (is_rank(c))
    m.to.rank = c - '1';
  else
    printf("BAD 4th RANK-FILE CHAR IN READ_MOVE @ char c = %c (%x)\n",c,c);

  c = next_char();

  if (c == '=') {
    m.promotes = TRUE;
    if ((m.promotep = p_from_c(next_char())) != None)
      c = next_char();
    else
      printf("BAD 2nd PROMOTION READ_MOVE @ char c = %c (%x)\n", c, c);
  }

  switch (c){ /* no breaks! */
    case '#':
      m.mate = TRUE;
    case '+':
      m.check = TRUE;
      c = next_char(); /* we need to check the next char is really a ' ' */
    case ' ':
      if (c != ' ') /* is it really? */
        printf("BAD 3rd END CHAR IN READ_MOVE @ char c = %c (%x)\n",c,c);
      return m;
    default:
      printf("NO END CHAR IN READ_MOVE @ char c = %c (%x)\n",c,c);
   }
  printf("NOT A COMPLETE MOVE IN READ_MOVE @ c = %c\n",c);
  return m;
}

char c_from_p(piece p){
  switch (p) {
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
    case Pawn:
      return 'P';
    default:
      return '_';
  }
}
void print_move_terse(move m){

  printf("%c",c_from_p(m.p));

  if (m.d.file == NOT_YET)
    printf("_");
  else
    printf("%c",m.d.file + 'a');

  if (m.d.rank == NOT_YET)
    printf("_");
  else
    printf("%c",m.d.rank + '1');

  if (m.to.file == NOT_YET)
    printf("_");
  else
    printf("%c",m.to.file + 'a');

  if (m.to.rank == NOT_YET)
    printf("_");
  else
    printf("%c",m.to.rank + '1');

  if (m.takes == FALSE)
    printf("_");
  else
    printf("x");

  if (m.mate)
    printf("#");
  else if (m.check)
    printf("+");
  else
    printf("_");

  if (m.promotes)
    printf("%c",c_from_p(m.promotep));
  else
    printf("_");

  /* We can deduce this easily enough.
  if(m.castle && m.to.file == 'g' - 'a')
    printf("k");
  else if (m.castle && m.to.file == 'c' - 'a')
    printf("q");
  else
    printf("_");
  */
}
void print_move_english(move m){

  if(m.wins == WHITE)
    printf("WHITE WINS\n");
  else if (m.wins == DRAW)
    printf("DRAW\n");
  else if (m.wins == BLACK)
    printf("BLACK WINS\n");
  if (m.wins != FALSE)
    return;

  printf("%s",piece_name(m.p));

  if (m.castle){
    printf(" castles");
    if(m.to.file == 'g' - 'a')
      printf(" kingside");
    else if (m.to.file == 'c' - 'a')
      printf(" queenside");
    else
      printf("BAD CASTLE MOVE");
    printf( " (that is, from the %c file to the %c file)\n",m.d.file + 'a', m.to.file + 'a');
    return;
  }
  if (m.takes)
    printf(" takes ");
  else
    printf(" moves to ");
  printf("%c%c",m.to.file + 'a',m.to.rank + '1');
  if (m.d.rank != NOT_YET && m.d.file != NOT_YET)
    printf(" from %c%c",m.d.file + 'a',m.d.rank + '1');
  else if (m.d.file != NOT_YET)
    printf(" from the %c file", m.d.file + 'a');
  else if (m.d.rank != NOT_YET)
    printf(" from the %c file", m.d.rank + '1');

  if (m.check)
    printf(", check");
  if (m.mate)
    printf("mate");

  printf("\n");
}


int main(){

  int plys = 0;
  int move_num;
  move m;
  m.over = FALSE;
  while(!m.over) {
    plys = 0;
    do {
      if (plys % 2 == 0) {
        m = read_move(); /* this is the move_num normally, but in some cases the win record */
      }
      if (m.move_num != 0) {
        move_num = m.move_num;
      }
      if (m.wins == WHITE){
        printf("W");
        break;
      }else if (m.wins == DRAW){
        printf("D");
        break;
      }else if (m.wins == BLACK){
        printf("B");
        break;
      }else{
        m = read_move();
        if (m.wins == WHITE){
          printf("W");
          break;
        }else if (m.wins == DRAW){
          printf("D");
          break;
        }else if (m.wins == BLACK){
          printf("B");
          break;
        }
      }
      if (m.over)
        break;
      if(plys != 0)
        printf("\t");
      printf("%03d", move_num);
      printf("%c", plys % 2 ? 'B' : 'W');
      print_move_terse(m);
    } while (plys++ < 1000 && m.wins == FALSE);
    printf("\n");
  }
}
