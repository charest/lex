// re2c $INPUT -o $OUTPUT
#include "errors.hpp"
#include "lex.hpp"
#include "stream.hpp"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <sstream>

/*!conditions:re2c*/

namespace lex {

std::tuple<int,stream_pos_t,int> scan(
  stream_t & strm,
  std::string & str) 
{
  int err = 0;
  stream_pos_t pos;

#if 0
  auto & in = strm.in;
  std::streampos mar;
  int state = yycinit;
  int ty = LEX_UNK;

  //while (true) {
    str.clear();
    pos.begin = in.tellg();

  /*!re2c
    re2c:api= custom;
    re2c:api:style = free-form;
    re2c:define:YYCTYPE = char;
    re2c:define:YYPEEK = "in.peek()";
    re2c:define:YYSKIP = "{  if (in.eof()) return {err, pos, ty}; str += yych; std::cout << str << std::endl; in.ignore(); pos.end=in.tellg(); }";
    re2c:define:YYBACKUP = "mar = in.tellg();";
    re2c:define:YYRESTORE = "in.seekg(mar);";
    re2c:define:YYGETCONDITION = "state";
    re2c:define:YYSETCONDITION = "state = @@;";
    re2c:define:YYSHIFT = "";
    re2c:yyfill:enable = 0;

    alpha = [a-zA-Z_];
    digit = [0-9];

    <*> * { std::cout << "LEAVE" << std::endl; return {err, pos, LEX_UNK}; }
    <ident> * { std::cout << "END" << std::endl; return {err, pos, ty}; }

    <init> "" / [1-9] :=> dec
    <init> "" / alpha :=> ident

    <dec> [0-9]? { ty = LEX_INT; goto yyc_dec; }
    <dec> *  { return {err, pos, LEX_INT}; }

    <ident> (alpha | digit) { ty = LEX_IDENT; goto yyc_ident; }

  */

  /*
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

    "" / [1-9]         { goto dec; }
  
    *
    {
      err += error(strm, "Unexpected character.");
      return {err, pos, LEX_UNK};
    }
  */

  //}
#endif

  return {err, pos, EOF};
}

int re2c_lex(stream_t & strm, lexed_t & lx) 
{
  int err, tok;
  stream_pos_t pos;
  std::string ident;
  size_t cur = 0;
  auto bufsize = strm.buffer.size();
  
  while(cur < bufsize) {
    std::tie(err, pos, tok) = scan(strm, ident);

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

  }

  return err;
}

} // lex
