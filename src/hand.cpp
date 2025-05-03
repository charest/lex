#include "errors.hpp"
#include "lex.hpp"
#include "stream.hpp"
#include "utils.hpp"

#include <cstdio>
#include <iostream>
#include <iomanip>

namespace lex {
  
 
//==============================================================================
/// Advance the file position
//==============================================================================
int advance(std::istream & in)
{ return in.get(); }
    

//==============================================================================
/// Read until a specified position
//==============================================================================
int get_until(std::istream & in, int c, std::string & str)
{
  int LastChar;
  while ((LastChar = advance(in)) != c)
    str += LastChar;
  return LastChar;
}
    
int a_or_ab(
  std::istream & in,
  int LastChar,
  int NextChar,
  int NextSym,
  int NextLabel,
  int & tok)
{
  if (NextChar == NextSym) {
    tok = NextLabel;
    advance(in);
  }
  else
    tok = LastChar;
  return advance(in);
}
  
//==============================================================================
/// gettok - Return the next token from standard input.
//==============================================================================
int gettok(
  stream_t & is,
  int & LastChar,
  int & tok,
  std::string & IdentifierStr)
{

  auto & in = is.in;
  auto NextChar = in.peek();
  int err = 0;
  IdentifierStr.clear();
  tok = LEX_UNK;
  
  //----------------------------------------------------------------------------
  // identifier: [a-zA-Z][a-zA-Z0-9]*
  if (std::isalpha(LastChar)) {

    IdentifierStr += LastChar;
    while (std::isalnum((LastChar = advance(in))) || LastChar=='_')
      IdentifierStr += LastChar;

    tok = LEX_IDENT;
    return err;
  }
  
  //----------------------------------------------------------------------------
  // Number: [0-9.]+

  if (std::isdigit(LastChar) || (LastChar == '.' && std::isdigit(NextChar))) {

    // read first part of number
    int numDec = (LastChar == '.');
    do {
      IdentifierStr += LastChar;
      LastChar = advance(in);
      auto has_dec = (LastChar == '.');
      if (numDec == 1 && has_dec)
        err += error( is, "Multiple '.' encountered in real" );
      numDec += has_dec;
    } while (std::isdigit(LastChar) || LastChar == '.');

    bool is_float = numDec;

    if (LastChar == 'e' || LastChar == 'E') {
      is_float = true;
      // eat e/E
      IdentifierStr += LastChar;
      LastChar = advance(in);
      // make sure next character is sign or number
      auto isSign = (LastChar == '+') || (LastChar == '-');
      if (!isSign && !std::isdigit(LastChar))
        err += error( is, "Digit or +/- must follow exponent" );
      // eat sign or number
      IdentifierStr += LastChar;
      LastChar = advance(in);
      // if it was a sign, there has to be a number
      if (isSign && !std::isdigit(LastChar))
        err += error( is, "Digit must follow exponent sign" );
      // only numbers should follow
      while (std::isdigit(LastChar)) {
        IdentifierStr += LastChar;
        LastChar = advance(in);
      }
    }
    tok = is_float ? LEX_REAL : LEX_INT;
    return err;
  }

  switch (LastChar) {

  //----------------------------------------------------------------------------
  // Comment until end of line.
  case '#':
  
    do
      LastChar = advance(in);
    while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

    tok = LEX_COMMENT;
    return err;
  
  
  //----------------------------------------------------------------------------
  // string literal
  case '\"':

    LastChar = get_until(in, '\"', IdentifierStr);
    LastChar = advance(in);
    tok = LEX_QUOTED;
    return err;
  
  //----------------------------------------------------------------------------
  // Operators

  case '+':
    LastChar = a_or_ab(in, LastChar, NextChar, '=', LEX_ADD_EQ, tok);
    return err;
  
  case '-':
    LastChar = a_or_ab(in, LastChar, NextChar, '=', LEX_SUB_EQ, tok);
    return err;
  
  case '*':
    LastChar = a_or_ab(in, LastChar, NextChar, '=', LEX_MUL_EQ, tok);
    return err;
  
  case '/':
    LastChar = a_or_ab(in, LastChar, NextChar, '=', LEX_DIV_EQ, tok);
    return err;
  
  case '=':
    LastChar = a_or_ab(in, LastChar, NextChar, '=', LEX_EQUIV, tok);
    return err;
  
  case '!':
    LastChar = a_or_ab(in, LastChar, NextChar, '=', LEX_NE, tok);
    return err;
  
  case '<':
    LastChar = a_or_ab(in, LastChar, NextChar, '=', LEX_LE, tok);
    return err;
  
  case '>':
    LastChar = a_or_ab(in, LastChar, NextChar, '=', LEX_GE, tok);
    return err;
  
  //----------------------------------------------------------------------------
  // Check for end of file.  Don't eat the EOF.
  case EOF:

    tok = LEX_EOF;
    return err;
  
  }

  //----------------------------------------------------------------------------
  // Otherwise, just return the character as its ascii value.
  tok = LastChar;
  LastChar = advance(in);
  return err;
}

//==============================================================================
/// gettok - Return the next token from standard input.
//==============================================================================
int gettok(
  stream_t & is,
  int & last_char,
  int & tok,
  stream_pos_t & pos,
  std::string & identifier)
{
  auto & in = is.in;

  // Skip any whitespace.
  while (isspace(last_char))
    last_char = advance(in);

  pos.begin = in.tellg();
  auto err = gettok(is, last_char, tok, identifier);
  pos.end = in.tellg();
  
  return err;
}

//==============================================================================
// Main function to generate tokens from a stream
//==============================================================================
int hand_lex(stream_t & in, lexed_t & res)
{
  int err = 0;
  std::string identifier;
  int tok;
  int last_char = ' ';
  stream_pos_t pos;
  do
  {
    err += gettok(in, last_char, tok, pos, identifier);
    res.add( tok, pos, identifier );
  } while (tok!=LEX_EOF);
    
  return err;
}

} // namespace
