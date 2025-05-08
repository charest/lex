#ifndef CONTRA_LEXER_HPP
#define CONTRA_LEXER_HPP

#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#define FOR_LEX_IDENT_STATES(DO) \
  DO( LEX_IDENT,  "IDENT") \
  DO( LEX_INT,    "INT") \
  DO( LEX_REAL,   "REAL") \
  DO( LEX_OCTAL,  "OCTAL" ) \
  DO( LEX_HEX,    "HEX" ) \
  DO( LEX_QUOTED, "QUOTED") \
  DO( LEX_UNK,    "UNK")
#define FOR_LEX_OTHER_STATES(DO) \
  DO( LEX_COMMENT,"COMMENT") \
  DO( LEX_ADD_EQ, "+=") \
  DO( LEX_SUB_EQ, "-=") \
  DO( LEX_MUL_EQ, "*=") \
  DO( LEX_DIV_EQ, "/=") \
  DO( LEX_EQUIV,  "==") \
  DO( LEX_NE,     "!=") \
  DO( LEX_GE,     ">=") \
  DO( LEX_LE,     "<=") \
  DO( LEX_XOR_EQ, "^=") \
  DO( LEX_INC,    "++" ) \
  DO( LEX_DEC,    "--" ) \
  DO( LEX_EOF,    "EOF")

namespace lex {

struct stream_t;
struct stream_pos_t;

enum LexToks {
  _LEX_STATE_START_  = 255,
#define DEFINE_TOKS(name, str, ...) name,
  FOR_LEX_IDENT_STATES(DEFINE_TOKS)
  FOR_LEX_OTHER_STATES(DEFINE_TOKS)
#undef DEFINE_TOKS
};

static std::string lex_to_str(int tok)
{
  switch (tok) {
#define TOKS_CASE(name, str, ...) case name: return str;
  FOR_LEX_IDENT_STATES(TOKS_CASE)
  FOR_LEX_OTHER_STATES(TOKS_CASE)
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
  std::vector<std::ios::pos_type> line_start;

  std::string identifier_data;
  std::vector<int> identifier_offsets;
  std::vector<int> identifier_tokens;

  void add(int tok, stream_pos_t pos, const std::string & str = "");

  void line(std::ios::pos_type p) { line_start.push_back(p); }

  void lines( std::vector<std::ios::pos_type> & p)
  { line_start.insert( line_start.end(), p.begin(), p.end() ); }

  size_t numTokens() const { return tokens.size(); }
  size_t numIdentifiers() const { return identifier_offsets.size(); }

  int findIdentifier(int tok) const;
  std::string_view getIdentifierString(int i) const;
};

//==============================================================================
/// The state machine datatype
//==============================================================================
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


/// Main lexer function
int hand_lex(stream_t & stream, lexed_t & lx);

/// Main lexer function
machine_t make_fsm_table();
int fsm_lex(stream_t & stream, const machine_t & table, lexed_t & lx);

/// re2c lexer function
int re2c_lex(stream_t & stream, lexed_t & lx);

  
/// Dump lexer results
void print(std::ostream& os, const lexed_t & res);

} // namespace

#endif // CONTRA_LEXER_HPP
