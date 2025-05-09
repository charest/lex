#include "errors.hpp"
#include "lex.hpp"
#include "stream.hpp"
#include "utils.hpp"

#include <cstdio>
#include <iostream>
#include <iomanip>

namespace lex {

struct scanner_t {
  const std::string & buffer;  
  std::size_t cursor = 0;

  scanner_t(const std::string & buf) : buffer(buf) {}

  int peek(int i=0) 
  { return ((cursor+i) >= buffer.size()) ? -1 : buffer[cursor+i]; }

  void advance()
  { 
    cursor++;
  }
};
 

    
int a_or_ab(
  scanner_t & scanner,
  int NextSym,
  int NextLabel)
{
  auto tok = scanner.peek();
  scanner.advance();
  auto LastChar = scanner.peek();
  if (LastChar == NextSym) {
    tok = NextLabel;
    scanner.advance();
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
  auto LastChar = scanner.peek();
  int err = 0;
  IdentifierStr.clear();
  
  //----------------------------------------------------------------------------
  // identifier: [a-zA-Z][a-zA-Z0-9]*
  if (std::isalpha(LastChar)) {
     
    do {
      IdentifierStr += LastChar;
      scanner.advance();
      LastChar = scanner.peek();
    } while (std::isalnum(LastChar) || LastChar=='_');

    return {LEX_IDENT, err};
  }
  
  //----------------------------------------------------------------------------
  // Number: [0-9.]+

  if (std::isdigit(LastChar) || (LastChar == '.' && std::isdigit(scanner.peek(1)))) {

    // read first part of number
    int numDec = (LastChar == '.');
    do {
      IdentifierStr += LastChar;
      scanner.advance();
      LastChar = scanner.peek();
      auto has_dec = (LastChar == '.');
      if (numDec == 1 && has_dec)
        err += error( is, "Multiple '.' encountered in real", scanner.cursor );
      numDec += has_dec;
    } while (std::isdigit(LastChar) || LastChar == '.');

    bool is_float = numDec;

    if (LastChar == 'e' || LastChar == 'E') {
      is_float = true;
      // eat e/E
      IdentifierStr += LastChar;
      scanner.advance();
      LastChar = scanner.peek();
      // make sure next character is sign or number
      auto isSign = (LastChar == '+') || (LastChar == '-');
      if (!isSign && !std::isdigit(LastChar))
        err += error( is, "Digit or +/- must follow exponent", scanner.cursor );
      // eat sign or number
      IdentifierStr += LastChar;
      scanner.advance();
      LastChar = scanner.peek();
      // if it was a sign, there has to be a number
      if (isSign && !std::isdigit(LastChar))
        err += error( is, "Digit must follow exponent sign", scanner.cursor );
      // only numbers should follow
      while (std::isdigit(LastChar)) {
        IdentifierStr += LastChar;
        scanner.advance();
        LastChar = scanner.peek();
      }
    }
    auto tok = is_float ? LEX_REAL : LEX_INT;
    return {tok, err};
  }

  switch (LastChar) {

  //----------------------------------------------------------------------------
  // Comment until end of line.
  case '#':
  
    do {
      scanner.advance();
      LastChar = scanner.peek();
    } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

    return {LEX_COMMENT, err};
  
  
  //----------------------------------------------------------------------------
  // string literal
  case '\"':
      
    scanner.advance();
    LastChar = scanner.peek();

    while (LastChar != '\"') {
      IdentifierStr += LastChar;
      scanner.advance();
      LastChar = scanner.peek();
    }

    scanner.advance();
    
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
  scanner.advance();
  return {tok, err};
}

//==============================================================================
// Main function to generate tokens from a stream
//==============================================================================
int hand_lex(stream_t & in, lexed_t & res)
{
  int err = 0;
  std::string identifier;
  stream_pos_t pos;

  scanner_t scanner{in.buffer};
  
  while (true)
  {
    // Skip any whitespace.
    while (isspace(scanner.peek()))
      scanner.advance();
    
    // get the next token
    pos.begin = scanner.cursor;
    auto [tok, e] = gettok(in, scanner, identifier);
    err += e;
    pos.end = scanner.cursor;

    if (tok == EOF) break;

    res.add( tok, pos, identifier );
    
  }
    
  return err;
}

} // namespace
