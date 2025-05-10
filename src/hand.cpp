#include "errors.hpp"
#include "lex.hpp"
#include "stream.hpp"
#include "utils.hpp"

#include <cstdio>
#include <iostream>
#include <iomanip>

namespace lex {

std::tuple<int,size_t,int>
a_or_ab(
  std::string & buffer,
  size_t cur,
  int NextSym,
  int NextLabel,
  int err)
{
  auto tok = buffer[cur];
  auto LastChar = buffer[++cur];
  if (LastChar == NextSym)
    return {NextLabel, ++cur, err};
  else
    return {tok, cur, err};
}
  
//==============================================================================
/// gettok - Return the next token from standard input.
//==============================================================================
std::tuple<int,size_t,int>
gettok( stream_t & is, size_t cur )
{
  auto & buffer = is.buffer;
  auto LastChar = buffer[cur];
  int err = 0;
  
  //----------------------------------------------------------------------------
  // identifier: [a-zA-Z][a-zA-Z0-9]*
  if (std::isalpha(LastChar)) {
     
    do {
      LastChar = buffer[++cur];
    } while (std::isalnum(LastChar) || LastChar=='_');

    return {LEX_IDENT, cur, err};
  }
  
  //----------------------------------------------------------------------------
  // Number: [0-9.]+

  if (std::isdigit(LastChar) || (LastChar == '.' && std::isdigit(buffer[cur+1]))) {

    // read first part of number
    int numDec = (LastChar == '.');
    do {
      LastChar = buffer[++cur];
      auto has_dec = (LastChar == '.');
      if (numDec == 1 && has_dec)
        err += error( is, "Multiple '.' encountered in real", cur );
      numDec += has_dec;
    } while (std::isdigit(LastChar) || LastChar == '.');

    bool is_float = numDec;

    if (LastChar == 'e' || LastChar == 'E') {
      is_float = true;
      // eat e/E
      LastChar = buffer[++cur];
      // make sure next character is sign or number
      auto isSign = (LastChar == '+') || (LastChar == '-');
      if (!isSign && !std::isdigit(LastChar))
        err += error( is, "Digit or +/- must follow exponent", cur );
      // eat sign or number
      LastChar = buffer[++cur];
      // if it was a sign, there has to be a number
      if (isSign && !std::isdigit(LastChar))
        err += error( is, "Digit must follow exponent sign", cur );
      // only numbers should follow
      while (std::isdigit(LastChar)) {
        LastChar = buffer[++cur];
      }
    }
    auto tok = is_float ? LEX_REAL : LEX_INT;
    return {tok, cur, err};
  }

  switch (LastChar) {

  //----------------------------------------------------------------------------
  // Comment until end of line.
  case '#':
  
    do {
      LastChar = buffer[++cur];
    } while (LastChar != '\0' && LastChar != '\n' && LastChar != '\r');

    return {LEX_COMMENT, cur, err};
  
  
  //----------------------------------------------------------------------------
  // string literal
  case '\"':
      
    LastChar = buffer[++cur];

    while (LastChar != '\"')
      LastChar = buffer[++cur];

    return {LEX_QUOTED, ++cur, err};
  
  //----------------------------------------------------------------------------
  // Operators

  case '+': return a_or_ab(buffer, cur, '=', LEX_ADD_EQ, err);
  case '-': return a_or_ab(buffer, cur, '=', LEX_SUB_EQ, err);
  case '*': return a_or_ab(buffer, cur, '=', LEX_MUL_EQ, err);
  case '/': return a_or_ab(buffer, cur, '=', LEX_DIV_EQ, err);
  case '=': return a_or_ab(buffer, cur, '=', LEX_EQUIV, err);
  case '!': return a_or_ab(buffer, cur, '=', LEX_NE, err);
  case '<': return a_or_ab(buffer, cur, '=', LEX_LE, err);
  case '>': return a_or_ab(buffer, cur, '=', LEX_GE, err);
  
  }

  //----------------------------------------------------------------------------
  // Otherwise, just return the character as its ascii value.
  return {LastChar, ++cur, err};
}

//==============================================================================
// Main function to generate tokens from a stream
//==============================================================================
int hand_lex(stream_t & in, lexed_t & lx)
{
  int err = 0;
  size_t cur = 0;
  auto & buffer = in.buffer;
  auto bufsize = in.buffer.size();
  
  while (cur < bufsize)
  {
    // Skip any whitespace.
    while (isspace(buffer[cur])) cur++;

    if (cur >= bufsize) break;

    // get the next token
    auto beg = cur;
    int e, tok;
    std::tie(tok, cur, e) = gettok(in, cur);
    err += e;
    auto end = cur;

    stream_pos_t pos{beg, end};
    
    // remove quotes
    if (tok == LEX_QUOTED) {
      beg++;
      end--;
    }

    auto len = end - beg;

    switch (tok) {
    #define TOKS_CASE(name, str, ...) \
      case name: lx.add(tok, pos, buffer.substr(beg, len)); break;
    FOR_LEX_IDENT_STATES(TOKS_CASE)
    #undef TOKS_CASE
    
    #define TOKS_CASE(name, str, ...) case name: lx.add(tok, pos); break;
    FOR_LEX_OTHER_STATES(TOKS_CASE)
    #undef TOKS_CASE
    
    case 0 ... 255:
      lx.add(tok, pos);
      break;
    }
    
  }
    
  return err;
}

} // namespace
