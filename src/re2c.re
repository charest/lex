// re2c $INPUT -o $OUTPUT
#include "errors.hpp"
#include "lex.hpp"
#include "stream.hpp"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <sstream>

namespace lex {

#if 0
static std::pair<int,int> lex_str(std::istream & in)
{
  for (unsigned long u = q;; fprintf(stderr, "\\x%lx", u)) {
    in.tok = in.cur;
    /*!re2c
      re2c:api= custom;
      re2c:api:style = free-form;
      re2c:define:YYCTYPE = char;
      re2c:define:YYPEEK = "in.peek()";
      re2c:define:YYSKIP = "{ str += yych; in.ignore(); if (in.eof()) return {err, EOF}; }";
      re2c:define:YYBACKUP = "mar = in.tellg();";
      re2c:define:YYRESTORE = "in.seekg(mar);";
      re2c:yyfill:enable = 0;
      *                    { return false; }
      [^\n\\]              { u = in.tok[0]; if (u == q) break; continue; }
      "\\a"                { u = '\a'; continue; }
      "\\b"                { u = '\b'; continue; }
      "\\f"                { u = '\f'; continue; }
      "\\n"                { u = '\n'; continue; }
      "\\r"                { u = '\r'; continue; }
      "\\t"                { u = '\t'; continue; }
      "\\v"                { u = '\v'; continue; }
      "\\\\"               { u = '\\'; continue; }
      "\\'"                { u = '\''; continue; }
      "\\\""               { u = '"';  continue; }
      "\\?"                { u = '?';  continue; }
      "\\" [0-7]{1,3}      { lex_oct(in.tok, in.cur, u); continue; }
      "\\u" [0-9a-fA-F]{4} { lex_hex(in.tok, in.cur, u); continue; }
      "\\U" [0-9a-fA-F]{8} { lex_hex(in.tok, in.cur, u); continue; }
      "\\x" [0-9a-fA-F]+   { if (!lex_hex(in.tok, in.cur, u)) return false; continue; }
    */
  }
  fprintf(stderr, "%c", q);
  return true;
}
#endif

std::tuple<int,stream_pos_t,int> scan(
  stream_t & strm,
  std::vector<std::ios::pos_type> & lines,
  std::string & str) 
{

  auto & in = strm.in;
  std::streampos mar;
  int err = 0;
  stream_pos_t pos;

  while (true) {
    str.clear();
    pos.begin = in.tellg();

  /*!re2c
    re2c:api= custom;
    re2c:api:style = free-form;
    re2c:define:YYCTYPE = char;
    re2c:define:YYPEEK = "in.peek()";
    re2c:define:YYSKIP = "{ str += yych; in.ignore(); pos.end=in.tellg(); if (in.eof()) return {err, pos, EOF}; }";
    re2c:define:YYBACKUP = "mar = in.tellg();";
    re2c:define:YYRESTORE = "in.seekg(mar);";
    re2c:yyfill:enable = 0;

    // end of file
    end = "\x00";
    end { return {err, pos, EOF}; }

    // single line comment
    scm = "#" [^\n]* ("\n" | end);
    scm { return {err, pos, LEX_COMMENT}; }
 
    // whitespace
    wsp = [ \t\v\f]+;
    wsp { continue; }

    // character and string literals
    //['"] { if (!lex_str(in, in.cur[-1])) return false; continue; }

    quote = ["] [^"]* ["];
    quote { str.pop_back(); str.erase(0, 1); return {err, pos, LEX_QUOTED}; }


    dig = [0-9];
    let = [a-zA-Z_];
    hex = [a-fA-F0-9];
    any = [\000-\377];

    special =  [!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?];

    let (let|dig)*   { return {err, pos, LEX_IDENT}; }
    "\r\n"|"\n"      { lines.push_back(pos.end); continue; }

    special          { return {err, pos, str[0]}; }
    "+="             { return {err, pos, LEX_ADD_EQ}; }
    "-="             { return {err, pos, LEX_SUB_EQ}; }
    "*="             { return {err, pos, LEX_MUL_EQ}; }
    "/="             { return {err, pos, LEX_DIV_EQ}; }
    "++"             { return {err, pos, LEX_INC   }; }
    "--"             { return {err, pos, LEX_DEC   }; }
    "<="             { return {err, pos, LEX_LE    }; }
    ">="             { return {err, pos, LEX_GE    }; }
    "=="             { return {err, pos, LEX_EQUIV }; }
    "!="             { return {err, pos, LEX_NE    }; }
	
    ("0" [xX] hex+)  { return {err, pos, LEX_HEX  }; }
    ("0" [0-7]+)     { return {err, pos, LEX_OCTAL}; }
    "0" | [1-9] (dig+)?  { return {err, pos, LEX_INT  }; }
    (dig+)? [.] dig+ ([eE][+-]?dig+)? { return {err, pos, LEX_REAL }; }
  
    *
    {
      err += error(strm, "Unexpected character.");
      return {err, pos, LEX_UNK};
    }
  */

  }
  return {err, pos, EOF};
}

int re2c_lex(stream_t & strm, lexed_t & lx) 
{
  int err, tok;
  stream_pos_t pos;
  std::string ident;
  auto & in = strm.in;

  do {
    std::tie(err, pos, tok) = scan(strm, lx.line_start, ident);

    switch (tok) {
    #define TOKS_CASE(name, str, ...) case name: lx.add(tok, pos, ident); break;
    FOR_LEX_IDENT_STATES(TOKS_CASE)
    #undef TOKS_CASE
    
    #define TOKS_CASE(name, str, ...) case name: lx.add(tok, pos); break;
    FOR_LEX_OTHER_STATES(TOKS_CASE)
    #undef TOKS_CASE

    case 0 ... 255:
      lx.add(tok, pos);
      break;
    }

  } while (tok != EOF);
  return err;
}

} // lex
