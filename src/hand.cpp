#include "errors.hpp"
#include "lex.hpp"
#include "stream.hpp"
#include "utils.hpp"

#include <cstdio>
#include <iostream>
#include <iomanip>

namespace lex {

struct scanner_t {
  std::istream & in;  
  int LastChar = ' ';
  int NextChar = ' ';
  std::ios::pos_type LastPos = std::ios::beg;
  std::ios::pos_type NextPos = std::ios::beg;

  std::vector<std::ios::pos_type> LineStart;

  scanner_t(std::istream & instr) : in(instr) {}

  int peek() { return in.peek(); }

  int advance()
  { 
    LastPos = NextPos;
    LastChar = in.get();
    NextPos = in.tellg();
    if (LastChar == '\n') LineStart.push_back(in.tellg());
    return LastChar;
  }
};
 

    
int a_or_ab(
  scanner_t & scanner,
  int NextSym,
  int NextLabel)
{
  auto tok = scanner.LastChar;
  auto LastChar = scanner.advance();
  if (LastChar == NextSym) {
    tok = NextLabel;
    LastChar = scanner.advance();
  }
  return tok;
}
  
//==============================================================================
/// gettok - Return the next token from standard input.
//==============================================================================
std::pair<int,int> gettok(
  stream_t & is,
  scanner_t & scanner,
  std::string & IdentifierStr)
{
  auto LastChar = scanner.LastChar;
  int err = 0;
  IdentifierStr.clear();
  
  //----------------------------------------------------------------------------
  // identifier: [a-zA-Z][a-zA-Z0-9]*
  if (std::isalpha(LastChar)) {
     
    while (std::isalnum(LastChar) || LastChar=='_') {
      IdentifierStr += LastChar;
      LastChar = scanner.advance();
    }

    return {LEX_IDENT, err};
  }
  
  //----------------------------------------------------------------------------
  // Number: [0-9.]+

  if (std::isdigit(LastChar) || (LastChar == '.' && std::isdigit(scanner.peek()))) {

    // read first part of number
    int numDec = (LastChar == '.');
    do {
      IdentifierStr += LastChar;
      LastChar = scanner.advance();
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
      LastChar = scanner.advance();
      // make sure next character is sign or number
      auto isSign = (LastChar == '+') || (LastChar == '-');
      if (!isSign && !std::isdigit(LastChar))
        err += error( is, "Digit or +/- must follow exponent" );
      // eat sign or number
      IdentifierStr += LastChar;
      LastChar = scanner.advance();
      // if it was a sign, there has to be a number
      if (isSign && !std::isdigit(LastChar))
        err += error( is, "Digit must follow exponent sign" );
      // only numbers should follow
      while (std::isdigit(LastChar)) {
        IdentifierStr += LastChar;
        LastChar = scanner.advance();
      }
    }
    auto tok = is_float ? LEX_REAL : LEX_INT;
    return {tok, err};
  }

  switch (LastChar) {

  //----------------------------------------------------------------------------
  // Comment until end of line.
  case '#':
  
    do
      LastChar = scanner.advance();
    while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

    return {LEX_COMMENT, err};
  
  
  //----------------------------------------------------------------------------
  // string literal
  case '\"':

    do { 
      LastChar = scanner.advance();
      IdentifierStr += LastChar;
    } while (LastChar!= '\"');
    IdentifierStr.pop_back();

    LastChar = scanner.advance();
    
    return {LEX_QUOTED, err};
  
  //----------------------------------------------------------------------------
  // Operators

  case '+':
    return {a_or_ab(scanner, '=', LEX_ADD_EQ), err};
  
  case '-':
    return {a_or_ab(scanner, '=', LEX_SUB_EQ), err};
  
  case '*':
    return {a_or_ab(scanner, '=', LEX_MUL_EQ), err};
  
  case '/':
    return {a_or_ab(scanner, '=', LEX_DIV_EQ), err};
  
  case '=':
    return {a_or_ab(scanner, '=', LEX_EQUIV), err};
  
  case '!':
    return {a_or_ab(scanner, '=', LEX_NE), err};
  
  case '<':
    return {a_or_ab(scanner, '=', LEX_LE), err};
  
  case '>':
    return {a_or_ab(scanner, '=', LEX_GE), err};
  
  //----------------------------------------------------------------------------
  // Check for end of file.  Don't eat the EOF.
  case EOF:

    return {EOF, err};
  
  }

  //----------------------------------------------------------------------------
  // Otherwise, just return the character as its ascii value.
  auto tok = LastChar;
  LastChar = scanner.advance();
  return {tok, err};
}

//==============================================================================
// Main function to generate tokens from a stream
//==============================================================================
int hand_lex(stream_t & in, lexed_t & res)
{
  if (!in.in) return error(in, "File does not exist.");

  int err = 0;
  std::string identifier;
  stream_pos_t pos;

  scanner_t scanner{in.in};
  scanner.advance();
  
  while (true)
  {
    // Skip any whitespace.
    while (isspace(scanner.LastChar))
      scanner.advance();
    
    // get the next token
    pos.begin = scanner.LastPos;
    auto [tok, e] = gettok(in, scanner, identifier);
    err += e;
    pos.end = scanner.LastPos;

    if (scanner.LineStart.size()) {
      res.lines( scanner.LineStart );
      scanner.LineStart.clear();
    }

    if (tok == EOF) break;

    res.add( tok, pos, identifier );
    
  }
    
  return err;
}

} // namespace
