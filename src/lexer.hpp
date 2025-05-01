#ifndef CONTRA_LEXER_HPP
#define CONTRA_LEXER_HPP

#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace lex {

struct stream_t;
struct stream_pos_t;

enum LexToks {
  LEX_IDENT = 256,
  LEX_INT,
  LEX_REAL,
  LEX_COMMENT,
  LEX_QUOTED,
  LEX_ADD_EQ,
  LEX_SUB_EQ,
  LEX_MUL_EQ,
  LEX_DIV_EQ,
  LEX_EQUIV,
  LEX_NE,
  LEX_GE,
  LEX_LE,
  LEX_UNK,
  LEX_EOF,
};

static std::string lex_to_str(int tok)
{
  switch (tok) {
  case LEX_UNK:    return "UNK";
  case LEX_IDENT:  return "IDENT";
  case LEX_INT:    return "INT";
  case LEX_REAL:   return "REAL";
  case LEX_COMMENT:return "COMMENT";
  case LEX_QUOTED: return "QUOTED";
  case LEX_ADD_EQ: return "ADD_EQ";
  case LEX_SUB_EQ: return "SUB_EQ";
  case LEX_MUL_EQ: return "MUL_EQ";
  case LEX_DIV_EQ: return "DIV_EQ";
  case LEX_EQUIV:  return "EQUIV";
  case LEX_NE:     return "NE";
  case LEX_GE:     return "GE";
  case LEX_LE:     return "LE";
  case LEX_EOF:    return "EOF";
  case 0 ... 255:  return std::string(1, tok);
  default:         return "Error";
  };
}

//==============================================================================
/// The lexer return datatype
//==============================================================================
struct lexed_t {
  std::vector<int> tokens;
  std::vector<stream_pos_t> token_pos;

  std::unordered_map<std::string, int> identifier_map;
  std::vector<std::string_view> identifiers;
  std::unordered_map<int, int> token_to_identifier;

  void add(int tok, stream_pos_t pos, const std::string & str = "");

  size_t numTokens() const { return tokens.size(); }
  size_t numIdentifiers() const { return identifiers.size(); }

  int findIdentifier(int tok) const;
  std::string_view getIdentifierString(int i) const;
};

/// Main lexer function
int hand_lex(stream_t & stream, lexed_t & lx);
  
/// Dump lexer results
void print(std::ostream& os, const lexed_t & res);

} // namespace

#endif // CONTRA_LEXER_HPP
