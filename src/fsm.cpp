#include "stream.hpp"
#include "lex.hpp"

#include <cstdlib>
#include <cctype>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#define FOR_FSM_CLASSES(DO) \
  DO( C_WHITE,   "WHITE")   /* space, tab */ \
  DO( C_CR,      "CR") \
  DO( C_LF,      "LF") \
  DO( C_EQUABLE, "EQUABLE") /*   ^ !     */         \
  DO( C_EQUAL,   "EQUAL")   /*   = \     */         \
  DO( C_ALPHA,   "ALPHA")   /*   a-zA-Z_ */         \
  DO( C_X,       "X")       /*   x X     */         \
  DO( C_ZERO,    "ZERO")    /*   0       */         \
  DO( C_DIGIT,   "DIGIT")   /*   0-9     */         \
  DO( C_PLUS,    "PLUS")    /*   +       */         \
  DO( C_AND,     "AND")     /*   &       */         \
  DO( C_OR,      "OR")      /*   |       */         \
  DO( C_HASH,    "HASH")    /*   #       */         \
  DO( C_DASH,    "DASH")    /*   -       */         \
  DO( C_GREAT,   "GREAT")   /*   >       */         \
  DO( C_LESS,    "LESS")    /*   <       */         \
  DO( C_SLASH,   "SLASH")   /*   /       */         \
  DO( C_STAR,    "STAR")    /*   *       */         \
  DO( C_MISC,    "MISC")    /* all other symbols */ \
  DO( C_DOT,     "DOT")     /*   .       */         \
  DO( C_SQUOTE,  "SQUOTE")  /*   '       */         \
  DO( C_DQUOTE,  "DQUOTE")  /*   "       */         \
  DO( C_EOF, "")

namespace lex {

enum FSM_CLASS
{
#define DEFINE_TOKS(name, str, ...) name,
  FOR_FSM_CLASSES(DEFINE_TOKS)
#undef DEFINE_TOKS
  C_SIZE
};

struct machine_t {
  int rows = 0, cols = 0;
  std::vector<int> table;
  
  void resize(int nstate, int nclass)
  {
    rows = nstate;
    cols = nclass;
    table.resize(nstate*nclass);
  }
  void fill(int v)
  { std::fill(table.begin(), table.end(), v); }

  auto operator()(int s, int c) const
  { return table[s*cols + c]; }
  
  auto & operator()(int s, int c)
  { return table[s*cols + c]; }
  
  void setRow(int r, int s)
  {
    for (int c=0; c<cols; ++c)
      table[r*cols + c] = s;
  }
};

machine_t stateTable;



/**
* FUNCTION: Get_FSM_Col
* USE: Determines the state of the type of character being examined.
* @param currentChar - A character.
* @return - Returns the state of the type of character being examined.
*/
int Get_FSM_Col(char c)
{
  switch (c) {
  case ('\r'): return C_CR;
  case ('\n'): return C_LF;
  case ('0'):  return C_ZERO;
  case ('x'):  case ('X'): return C_X;
  case ('_'):  return C_ALPHA;
  case ('^'):  case ('!'): return C_EQUABLE;
  case ('='):  return C_EQUAL;
  case ('+'):  return C_PLUS;
  case ('&'):  return C_AND;
  case ('|'):  return C_OR;
  case ('#'):  return C_HASH;
  case ('-'):  return C_DASH;
  case ('>'):  return C_GREAT;
  case ('<'):  return C_LESS;
  case ('/'):  return C_SLASH;
  case ('*'):  return C_STAR;
  case ('.'):  return C_DOT;
  case ('\''):  return C_SQUOTE;
  case ('"'):  return C_DQUOTE;
  }

  if (isspace(c)) return C_WHITE;
  if (isdigit(c)) return C_DIGIT;
  if (isalpha(c)) return C_ALPHA;
  if (ispunct(c)) return C_MISC;
  return S_UNK;
}// end of Get_FSM_Col

/**
* FUNCTION: GetLexemeName
* USE: Returns the string equivalent of an integer lexeme token type.
* @param lexeme - An integer lexeme token type.
* @return - An std::string string representing the name of the integer
*        lexeme token type.
*/
std::string GetLexemeName(int lexeme)
{
  switch(lexeme)
  {
#define TOKS_CASE(name, str, ...) case name: return str;
    FOR_FSM_STATES(TOKS_CASE)
#undef TOKS_CASE
    default:      return "ERROR";
  }
}

/**
* FUNCTION: Lexer
* USE: Parses the "expression" string using the Finite State Machine to
*     isolate each individual token and lexeme name in the expression.
* @param expression - A std::string containing text.
* @return - Returns a vector containing the tokens found in the string
*/
int Lexer(std::string expression, lexed_t lx)
{
    char currentChar = ' ';
    int col = S_REJECT;
    int currentState = S_REJECT;
    int prevState = S_REJECT;
    std::string currentToken = "";

    // use an FSM to parse the expression
    for(unsigned x = 0; x < expression.length();)
    {
        currentChar = expression[x];
        //std:: cout << currentChar << " " << GetLexemeName(currentState) << " ";

        // get the column number for the current character
        col = Get_FSM_Col(currentChar);
        //std:: cout << " col " << GetClassName(col) << std::endl;;

        /* ========================================================

            THIS IS WHERE WE CHECK THE FINITE STATE MACHINE TABLE
               USING THE "col" VARIABLE FROM ABOVE ^

          ========================================================= */

        // get the current state of the expression
        currentState = stateTable(currentState, col);
        //std:: cout << " -> " << GetLexemeName(currentState) << std::endl;;

        /* ===================================================

          THIS IS WHERE WE CHECK FOR A SUCESSFUL PARSE
           - If the current state of the expression == REJECT
             (the starting state), then we have sucessfully parsed
             a token.

            - ELSE we continue trying to find a sucessful token

            =================================================== */
        if(currentState == S_REJECT)
        {
            if(prevState != S_SPACE) // we dont care about whitespace
              lx.add(prevState, 0, currentToken);
            currentToken = "";
        }
        else
        {
            currentToken += currentChar;
            ++x;
        }
        prevState = currentState;

    }
    // this ensures the last token gets saved when
    // we reach the end of the loop (if a valid token exists)
    if(currentState != S_SPACE && currentToken != "")
      // ^^ we dont care about whitespace
      lx.add(currentState, 0, currentToken);
    
    return 0;
}// end of Lexer



int fsm_lex(stream_t & in, lexed_t & lx)
{
  stateTable.resize(FSM_NUM_STATES, C_SIZE);
  stateTable.fill(S_REJECT);
  
  stateTable(S_REJECT, C_WHITE) = S_SPACE;

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
  
  stateTable(S_REJECT, C_DOT  ) = S_REAL;
  stateTable(S_REAL  , C_DIGIT) = S_REAL;
  stateTable(S_REAL  , C_ZERO ) = S_REAL;
  stateTable(S_REAL  , C_DOT  ) = S_UNK;
  
  stateTable.setRow(S_UNK, S_UNK);
  stateTable(S_UNK, C_WHITE) = S_REJECT;

  stateTable(S_REJECT, C_EQUAL) = S_EQUAL;
  stateTable(S_EQUAL , C_EQUAL) = S_EQUIV;
  
  stateTable(S_REJECT, C_PLUS ) = S_ADD;
  stateTable(S_ADD   , C_PLUS ) = S_INC;
  stateTable(S_ADD   , C_EQUAL) = S_ADD_EQ;
  
  stateTable(S_REJECT, C_DASH ) = S_SUB;
  stateTable(S_SUB   , C_DASH ) = S_DEC;
  stateTable(S_SUB   , C_EQUAL) = S_SUB_EQ;
  
  // declare variables
  std::string expression = "";

  // get data from user
  auto & infile = in.in;

  // use a loop to scan each line in the file
  while(!infile.eof())
  {
      // use the "Lexer" function to isolate integer, real, operator,
      // string, and unknown tokens
      auto err = Lexer(expression, lx);

  }

  lx.tokens.push_back( LEX_EOF );

  return 0;
}// end of main


} // namespace
