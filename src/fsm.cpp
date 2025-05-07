#include "stream.hpp"
#include "errors.hpp"
#include "lex.hpp"

#include <cstdlib>
#include <cctype>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#define FOR_FSM_TRANS_STATES(DO) \
  DO( S_REJECT     , "REJECT"      ) \
  DO( S_SPACE      , "SPACE"       ) \
  DO( S_NEWLINE    , "NEWLINE"     ) \
  DO( S_SEEN_DQUOTE, "SEEN_DQUOTE" ) \
  DO( S_SEEN_HASH  , "SEEN_HASH"   )

#define FOR_FSM_FINAL_ID_STATES(DO) \
  DO( S_INT   , "INT_LIT"  , LEX_INT    ) \
  DO( S_REAL  , "REAL_LIT" , LEX_REAL   ) \
  DO( S_ZERO  , "ZERO"     , LEX_INT    ) \
  DO( S_OCTAL , "OCTAL"    , LEX_OCTAL  ) \
  DO( S_HEX   , "HEX"      , LEX_HEX    ) \
  DO( S_IDENT , "IDENT"    , LEX_IDENT  ) \
  DO( S_EOF ,   "EOF"      , LEX_EOF    ) \
  DO( S_UNK ,   "UNK"      , LEX_UNK    )
  
#define FOR_FSM_OTHER_FINAL_STATES(DO) \
  DO( S_QUOTED, "QUOTED"   , LEX_QUOTED ) \
  DO( S_COMMENT, "COMMENT" , LEX_COMMENT )

#define FOR_FSM_FINAL_OP_STATES(DO) \
  DO( S_DOT   , "DOT"      , '.'        ) \
  DO( S_EQUAL , "EQUAL"    , '='        ) \
  DO( S_ADD   , "ADD"      , '+'        ) \
  DO( S_SUB   , "SUB"      , '-'        ) \
  DO( S_MUL   , "MUL"      , '*'        ) \
  DO( S_DIV   , "DIV"      , '/'        ) \
  DO( S_GT    , "GT"       , '>'        ) \
  DO( S_LT    , "LT"       , '<'        ) \
  DO( S_EQUIV , "EQUIV"    , LEX_EQUIV  ) \
  DO( S_ADD_EQ, "ADD_EQ"   , LEX_ADD_EQ ) \
  DO( S_SUB_EQ, "SUB_EQ"   , LEX_SUB_EQ ) \
  DO( S_MUL_EQ, "MUL_EQ"   , LEX_MUL_EQ ) \
  DO( S_DIV_EQ, "DIV_EQ"   , LEX_DIV_EQ ) \
  DO( S_GE    , "GE"       , LEX_GE     ) \
  DO( S_LE    , "LE"       , LEX_LE     ) \
  DO( S_INC   , "INC"      , LEX_INC    ) \
  DO( S_DEC   , "DEC"      , LEX_DEC    )
  
#define FOR_FSM_EXACT_STATES(DO) \
  DO( S_OP     , "OP"      ) \
  DO( S_EQUABLE, "EQUABLE" )

#define FOR_FSM_DECODE_STATES(DO) \
  DO( S_EQUABLE_EQ, "EQUABLE", "!=", LEX_NE, "^=", LEX_XOR_EQ )



#define FOR_FSM_ONE_CHAR_CLASSES(DO) \
  DO( C_CR,      "CR"    , '\r') \
  DO( C_LF,      "LF"    , '\n') \
  DO( C_EQUAL,   "EQUAL" , '=') \
  DO( C_ZERO,    "ZERO"  , '0') \
  DO( C_PLUS,    "PLUS"  , '+') \
  DO( C_AND,     "AND"   , '&') \
  DO( C_OR,      "OR"    , '|') \
  DO( C_HASH,    "HASH"  , '#') \
  DO( C_DASH,    "DASH"  , '-') \
  DO( C_GREAT,   "GREAT" , '>') \
  DO( C_LESS,    "LESS"  , '<') \
  DO( C_SLASH,   "SLASH" , '/') \
  DO( C_STAR,    "STAR"  , '*') \
  DO( C_DOT,     "DOT"   , '.') \
  DO( C_SQUOTE,  "SQUOTE", '\'') \
  DO( C_DQUOTE,  "DQUOTE", '\"') \
  DO( C_EOF,     "EOF"   , EOF)

#define FOR_FSM_TWO_CHAR_CLASSES(DO) \
  DO( C_EQUABLE, "EQUABLE", '^', '!') \
  DO( C_X,       "X"      , 'x', 'X')

#define FOR_FSM_IF_CLASSES(DO) \
  DO( C_WHITE,   "WHITE" , isspace) \
  DO( C_DIGIT,   "DIGIT" , isdigit) \
  DO( C_MISC,    "MISC"  , ispunct)
  
#define FOR_FSM_IF_CHAR_CLASSES(DO) \
  DO( C_ALPHA,   "ALPHA" , isalpha, '_')

namespace lex {

enum FSM_STATES {
#define DEFINE_TOKS(name, str, ...) name,
  FOR_FSM_TRANS_STATES(DEFINE_TOKS)
  FOR_FSM_FINAL_ID_STATES(DEFINE_TOKS)
  FOR_FSM_OTHER_FINAL_STATES(DEFINE_TOKS)
  FOR_FSM_FINAL_OP_STATES(DEFINE_TOKS)
  FOR_FSM_EXACT_STATES(DEFINE_TOKS)
  FOR_FSM_DECODE_STATES(DEFINE_TOKS)
#undef DEFINE_TOKS
  FSM_NUM_STATES
};

enum FSM_CLASS
{
#define DEFINE_TOKS(name, str, ...) name,
  FOR_FSM_ONE_CHAR_CLASSES(DEFINE_TOKS)
  FOR_FSM_TWO_CHAR_CLASSES(DEFINE_TOKS)
  FOR_FSM_IF_CLASSES(DEFINE_TOKS)
  FOR_FSM_IF_CHAR_CLASSES(DEFINE_TOKS)
#undef DEFINE_TOKS
  C_SIZE
};

std::string state_to_str(int tok)
{
  switch (tok) {
#define TOKS_CASE(name, str, ...) case name: return str;
  FOR_FSM_TRANS_STATES(TOKS_CASE)
  FOR_FSM_FINAL_ID_STATES(TOKS_CASE)
  FOR_FSM_OTHER_FINAL_STATES(TOKS_CASE)
  FOR_FSM_FINAL_OP_STATES(TOKS_CASE)
  FOR_FSM_EXACT_STATES(TOKS_CASE)
  FOR_FSM_DECODE_STATES(TOKS_CASE)
#undef TOKS_CASE
  default: return "Error";
  };
}

std::string class_to_str(int tok)
{
  switch (tok) {
#define TOKS_CASE(name, str, ...) case name: return str;
  FOR_FSM_ONE_CHAR_CLASSES(TOKS_CASE)
  FOR_FSM_TWO_CHAR_CLASSES(TOKS_CASE)
  FOR_FSM_IF_CLASSES(TOKS_CASE)
  FOR_FSM_IF_CHAR_CLASSES(TOKS_CASE)
#undef TOKS_CASE
  default: return "Error";
  };
}


int char_to_class(char c)
{
  switch (c) {

  #define TOKS_CASE(name, str, ch) case ch: return name;
  FOR_FSM_ONE_CHAR_CLASSES(TOKS_CASE)
  #undef TOKS_CASE
  
  #define TOKS_CASE(name, str, ch1, ch2) case ch1: case ch2: return name;
  FOR_FSM_TWO_CHAR_CLASSES(TOKS_CASE)
  #undef TOKS_CASE
  
  #define TOKS_CASE(name, str, ch1, ch2) case ch2: return name;
  FOR_FSM_IF_CHAR_CLASSES(TOKS_CASE)
  #undef TOKS_CASE

  }

  #define TOKS_CASE(name, str, ifch, ...) if (ifch(c)) return name;
  FOR_FSM_IF_CHAR_CLASSES(TOKS_CASE)
  FOR_FSM_IF_CLASSES(TOKS_CASE)
  #undef TOKS_CASE
  
  return S_UNK;
}// end of Get_FSM_Col

machine_t make_fsm_table() {
  machine_t stateTable;

  stateTable.resize(FSM_NUM_STATES, C_SIZE);
  stateTable.fill(S_REJECT);
  
  stateTable(S_REJECT, C_LF) = S_NEWLINE;
  
  stateTable(S_REJECT, C_WHITE) = S_SPACE;
  stateTable(S_SPACE,  C_WHITE) = S_SPACE;

  stateTable(S_REJECT, C_ALPHA) = S_IDENT;
  stateTable(S_IDENT , C_ALPHA) = S_IDENT;
  stateTable(S_IDENT , C_X    ) = S_IDENT;
  stateTable(S_IDENT , C_DIGIT) = S_IDENT;
  stateTable(S_IDENT , C_ZERO ) = S_IDENT;
  
  stateTable(S_REJECT, C_X) = S_IDENT;
  
  stateTable(S_REJECT, C_DIGIT) = S_INT;
  stateTable(S_INT   , C_DIGIT) = S_INT;
  stateTable(S_INT   , C_ZERO ) = S_INT;
  stateTable(S_INT   , C_DOT  ) = S_REAL;
  stateTable(S_INT   , C_X    ) = S_UNK;
  
  stateTable(S_REJECT, C_ZERO ) = S_ZERO;
  stateTable(S_ZERO  , C_DIGIT) = S_OCTAL;
  stateTable(S_ZERO  , C_ZERO ) = S_OCTAL;
  stateTable(S_OCTAL , C_DIGIT) = S_OCTAL;
  stateTable(S_OCTAL , C_ZERO ) = S_OCTAL;
  stateTable(S_OCTAL , C_X    ) = S_UNK;
  stateTable(S_ZERO  , C_X    ) = S_HEX;
  stateTable(S_HEX   , C_DIGIT) = S_HEX;
  stateTable(S_HEX   , C_ZERO ) = S_HEX;
  
  stateTable(S_REJECT, C_DOT  ) = S_DOT;
  stateTable(S_DOT   , C_DIGIT) = S_REAL;
  stateTable(S_DOT   , C_ZERO ) = S_REAL;
  stateTable(S_REAL  , C_DIGIT) = S_REAL;
  stateTable(S_REAL  , C_ZERO ) = S_REAL;
  stateTable(S_REAL  , C_DOT  ) = S_UNK;
  
  stateTable(S_REJECT, C_EQUAL) = S_EQUAL;
  stateTable(S_EQUAL , C_EQUAL) = S_EQUIV;
  
  stateTable(S_REJECT, C_PLUS ) = S_ADD;
  stateTable(S_ADD   , C_PLUS ) = S_INC;
  stateTable(S_ADD   , C_EQUAL) = S_ADD_EQ;
  
  stateTable(S_REJECT, C_DASH ) = S_SUB;
  stateTable(S_SUB   , C_DASH ) = S_DEC;
  stateTable(S_SUB   , C_EQUAL) = S_SUB_EQ;
  
  stateTable(S_REJECT, C_STAR ) = S_MUL;
  stateTable(S_MUL   , C_EQUAL) = S_MUL_EQ;
  
  stateTable(S_REJECT, C_SLASH) = S_DIV;
  stateTable(S_DIV   , C_EQUAL) = S_DIV_EQ;
  
  stateTable(S_REJECT, C_GREAT) = S_GT;
  stateTable(S_GT    , C_EQUAL) = S_GE;
  
  stateTable(S_REJECT, C_LESS ) = S_LT;
  stateTable(S_LT    , C_EQUAL) = S_LE;
  
  stateTable(S_REJECT, C_MISC ) = S_OP;

  stateTable(S_REJECT, C_EQUABLE) = S_EQUABLE;
  stateTable(S_EQUABLE, C_EQUAL ) = S_EQUABLE_EQ;
  
  stateTable(S_REJECT, C_DQUOTE) = S_SEEN_DQUOTE;
  stateTable.setRow(S_SEEN_DQUOTE, S_SEEN_DQUOTE);
  stateTable(S_SEEN_DQUOTE, C_DQUOTE) = S_QUOTED;
  stateTable(S_SEEN_DQUOTE, C_EOF)    = S_REJECT;
  
  stateTable(S_REJECT, C_HASH) = S_SEEN_HASH;
  stateTable.setRow(S_SEEN_HASH, S_SEEN_HASH);
  stateTable(S_SEEN_HASH, C_LF)  = S_COMMENT;
  stateTable(S_SEEN_HASH, C_EOF) = S_COMMENT;
  
  stateTable.setRow(S_UNK, S_UNK);
  stateTable(S_UNK, C_WHITE) = S_REJECT;
  stateTable(S_UNK, C_EOF) = S_REJECT;
  stateTable(S_UNK, C_LF) = S_REJECT;
  
  return stateTable;
}

int fsm_lex(stream_t & in, const machine_t & table, lexed_t & lx)
{
  // get data from user
  auto & infile = in.in;
  
  if (!infile) return error(in, "File does not exist.");
  
  // declare variables
  int err = 0;
  int col;
  auto currChar = ' ';
  int currState = S_REJECT;
  int prevState = S_REJECT;
  auto prevPos = infile.tellg();
  auto currPos = prevPos;
  std::string currToken = "";
    

  // use a loop to scan each line in the file
  while(!infile.eof())
  {
    auto begPos = prevPos;

    do {
      prevState = currState;
      prevPos = currPos;
      
      currChar = infile.get();
      currToken += currChar;
      currPos = infile.tellg();
      
      // get the column number for the curr character
      col = char_to_class(currChar);

      // get the curr state of the expression
      currState = table(currState, col);

    } while (currState != S_REJECT);

    // If the curr state of the expression == REJECT
    // (the starting state), then we have sucessfully parsed
    // a token.
      
    currToken.pop_back(); // last char is rejected
    stream_pos_t pos{begPos, prevPos};
    
    if (prevState == S_UNK) err += error(in, "Unknown string.", pos, lx.line_start);

    switch (prevState) {

      #define STATE_CASE(name, str) case name: lx.add(currToken[0], pos); break;
      FOR_FSM_EXACT_STATES(STATE_CASE)
      #undef STATE_CASE

      #define STATE_CASE(name, str, lstate) case name: lx.add(lstate, pos, currToken); break;
      FOR_FSM_FINAL_ID_STATES(STATE_CASE)
      #undef STATE_CASE

      #define STATE_CASE(name, str, lstate) case name: lx.add(lstate, pos); break;
      FOR_FSM_FINAL_OP_STATES(STATE_CASE)
      #undef STATE_CASE

      case S_QUOTED:
        lx.add(LEX_QUOTED, pos, currToken.substr(1, currToken.size()-2));
        break;
      
      case S_COMMENT:
        lx.add(LEX_COMMENT, pos);
        break;


      case S_EQUABLE_EQ:
        lx.add( currToken[0] == '!' ? LEX_NE : LEX_XOR_EQ, pos );
        break;

      case S_NEWLINE:
        lx.line(pos.end);
        break;

    } // switch

    // Reset the state/token
    currState = table(currState, col);
    currToken = currChar;
  }

  return err;
}// end of main


} // namespace
