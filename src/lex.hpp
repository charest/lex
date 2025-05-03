#ifndef CONTRA_LEXER_HPP
#define CONTRA_LEXER_HPP

#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#define FOR_LEX_STATES(DO) \
  DO( LEX_IDENT,  "IDENT") \
  DO( LEX_INT,    "INT") \
  DO( LEX_REAL,   "REAL") \
  DO( LEX_COMMENT,"COMMENT") \
  DO( LEX_QUOTED, "QUOTED") \
  DO( LEX_ADD_EQ, "ADD_EQ") \
  DO( LEX_SUB_EQ, "SUB_EQ") \
  DO( LEX_MUL_EQ, "MUL_EQ") \
  DO( LEX_DIV_EQ, "DIV_EQ") \
  DO( LEX_EQUIV,  "EQUIV") \
  DO( LEX_NE,     "NE") \
  DO( LEX_GE,     "GE") \
  DO( LEX_LE,     "LE") \
  DO( LEX_UNK,    "UNK") \
  DO( LEX_EOF,    "EOF")

#define FOR_FSM_STATES(DO) \
  DO( S_REJECT, "REJECT" ) \
  DO( S_INT, "INT_LIT" ) \
  DO( S_REAL, "REAL_LIT" ) \
  DO( S_ZERO, "ZERO" ) \
  DO( S_OCTAL, "OCTAL" ) \
  DO( S_HEX, "HEX" ) \
  DO( S_IDENT, "IDENT" ) \
  DO( S_EQUAL, "EQUAL" ) \
  DO( S_EQUIV, "EQUIV" ) \
  DO( S_ADD, "ADD" ) \
  DO( S_ADD_EQ, "ADD_EQ" ) \
  DO( S_INC, "INC" ) \
  DO( S_SUB, "SUB" ) \
  DO( S_SUB_EQ, "SUB_EQ" ) \
  DO( S_DEC, "DEC" ) \
  DO( S_OP, "OP" ) \
  DO( S_SPACE, "SPACE" ) \
  DO( S_UNK, "UNKNOWN" ) 

namespace lex {

struct stream_t;
struct stream_pos_t;

enum LexToks {
  _TOKS_START_  = 255,
#define DEFINE_TOKS(name, str, ...) name,
  FOR_LEX_STATES(DEFINE_TOKS)
#undef DEFINE_TOKS
};

enum FsmToks {
#define DEFINE_TOKS(name, str, ...) name,
  FOR_FSM_STATES(DEFINE_TOKS)
#undef DEFINE_TOKS
  FSM_NUM_STATES
};

static std::string lex_to_str(int tok)
{
  switch (tok) {
#define TOKS_CASE(name, str, ...) case name: return str;
  FOR_LEX_STATES(TOKS_CASE)
#undef TOKS_CASE
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

/// Main lexer function
int fsm_lex(stream_t & stream, lexed_t & lx);
  
/// Dump lexer results
void print(std::ostream& os, const lexed_t & res);

} // namespace

#endif // CONTRA_LEXER_HPP
