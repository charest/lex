#include <cstdlib>
#include <cctype>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

enum FSM_TRANS
{
  S_REJECT,
  S_INT,
  S_REAL,
  S_ZERO,
  S_OCTAL,
  S_HEX,
  S_IDENT,
  S_EQUAL,
  S_EQUIV,
  S_ADD,
  S_ADD_EQ,
  S_INC,
  S_SUB,
  S_SUB_EQ,
  S_DEC,
  S_OP,
  S_SPACE,
  S_UNK,
  S_SIZE
};
enum FSM_CLASS
{
  C_WHITE,   // space, tab
  C_CR,
  C_LF,
  C_EQUABLE, // ^ !
  C_EQUAL,   // =
  C_ALPHA,   // a-zA-Z_
  C_X,       // x X
  C_ZERO,    // 0
  C_DIGIT,   // 0-9
  C_PLUS,    // +
  C_AND,     // &
  C_OR,      // |
  C_HASH,    // #
  C_DASH,    // -
  C_GREATER, // >
  C_LESS,    // <
  C_SLASH,   // /
  C_STAR,    // *
  C_MISC,    // all other symbols
  C_DOT,  // .
  C_SQUOTE,  // '
  C_DQUOTE,  // "
  C_EOF,
  C_SIZE
};
// struct to hold token information
struct TokenType
{
    std::string token;
    int lexeme;
    std::string lexemeName;
};

// function prototypes
std::vector<TokenType> Lexer(std::string expression);
int Get_FSM_Col(char currentChar);
std::string GetLexemeName(int lexeme);

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

#if 0
int main(int argc, char * argv[])
{
  stateTable.resize(S_SIZE, C_SIZE);
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
  std::ifstream infile;
  std::string expression = "";
  std::vector<TokenType> tokens;

  // get data from user
  infile.open(argv[1]);

  if(infile.fail())
  {
    std::cout<<"\n** ERROR - the file \""<<argv[1]<<"\" cannot be found!\n\n";
    exit(1);
  }

  // use a loop to scan each line in the file
  while(getline(infile, expression))
  {
      // use the "Lexer" function to isolate integer, real, operator,
      // string, and unknown tokens
      tokens = Lexer(expression);

      // display the tokens to the screen
      for(unsigned x = 0; x < tokens.size(); ++x)
      {
        std::stringstream ss;
        ss << "\"" << tokens[x].token << "\"";
        std::cout << std::setw(8) << std::left << tokens[x].lexemeName;
        std::cout << std::setw(10) << ss.str() << std::endl;
      }
  }

  infile.close();

  return 0;
}// end of main

#endif

/**
* FUNCTION: Lexer
* USE: Parses the "expression" string using the Finite State Machine to
*     isolate each individual token and lexeme name in the expression.
* @param expression - A std::string containing text.
* @return - Returns a vector containing the tokens found in the string
*/
std::vector<TokenType> Lexer(std::string expression)
{
    TokenType access;
    std::vector<TokenType> tokens;
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
            {
                access.token = currentToken;
                access.lexeme = prevState;
                access.lexemeName = GetLexemeName(access.lexeme);
                tokens.push_back(access);
            }
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
    {// ^^ we dont care about whitespace
        access.token = currentToken;
        access.lexeme = currentState;
        access.lexemeName = GetLexemeName(access.lexeme);
        tokens.push_back(access);
    }
    return tokens;
}// end of Lexer

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
  case ('>'):  return C_GREATER;
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
    case S_INT:   return "INT";
    case S_REAL:  return "REAL";
    case S_ZERO:  return "ZERO";
    case S_OCTAL: return "OCTAL";
    case S_HEX:   return "HEX";
    case S_IDENT: return "IDENT";
    case S_EQUAL: return "EQUAL";
    case S_EQUIV: return "EQUIV";
    case S_ADD:   return "ADD";
    case S_ADD_EQ:return "ADD_EQ";
    case S_INC:   return "INC";
    case S_SUB:   return "SUB";
    case S_SUB_EQ:return "SUB_EQ";
    case S_DEC:   return "DEC";
    case S_OP:    return "OP";
    case S_SPACE: return "SPACE";
    case S_UNK:   return "UNK";
    case S_REJECT:return "REJECT";
    default:      return "ERROR";
  }
}// http://programmingnotes.org/
