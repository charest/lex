// re2c $INPUT -o $OUTPUT
#include "errors.hpp"
#include "lex.hpp"
#include "stream.hpp"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <sstream>

namespace lex {

std::tuple<int,char *, char *,int> scan(stream_t & strm, char * YYCURSOR)
{
  int err = 0;
  auto YYMARKER = YYCURSOR;
  auto start = YYCURSOR;
  auto & buffer = strm.buffer;
  auto buflen = buffer.size();
  auto bufbeg = &buffer[0];
  auto bufend = &buffer[buflen];

  while (YYCURSOR < bufend) {
    start = YYCURSOR;

  /*!re2c
    re2c:define:YYCTYPE  = char;
    re2c:yyfill:enable   = 0;

    *
    {
      err += error(strm, "Unexpected character.", YYCURSOR-bufbeg);
      return {err, start, YYCURSOR, LEX_UNK};
    }

    // whitespace
    wsp = [ \t\v\n\r]+;
    wsp { continue; }
    
    // single line comment
    scm = "#" [^\n\x00]*;
    scm { return {err, start, YYCURSOR, LEX_COMMENT}; }

    let = [a-zA-Z_];
    dig = [0-9];
    oct = "0" [0-7]*;
    dec = "0" | ([1-9][0-9]*);
    hex = '0x' [0-9a-fA-F]+;

    dec       { return {err, start, YYCURSOR, LEX_INT  }; }
    oct       { return {err, start, YYCURSOR, LEX_OCTAL}; }
    hex       { return {err, start, YYCURSOR, LEX_HEX  }; }

    // floating literals
    frc = [0-9]* "." [0-9]+ | [0-9]+ ".";
    exp = 'e' [+-]? [0-9]+;
    flt = (frc exp? | [0-9]+ exp);
    flt { return {err, start, YYCURSOR, LEX_REAL}; }

    quote = ["] [^"]* ["];
    quote            { return {err, start, YYCURSOR,    LEX_QUOTED}; }

    
    special =  [!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?];

    let (let|dig)*   { return {err, start, YYCURSOR,     LEX_IDENT}; }


    special          { return {err, start, YYCURSOR, *(YYCURSOR-1)}; }
    "+="             { return {err, start, YYCURSOR,    LEX_ADD_EQ}; }
    "-="             { return {err, start, YYCURSOR,    LEX_SUB_EQ}; }
    "*="             { return {err, start, YYCURSOR,    LEX_MUL_EQ}; }
    "/="             { return {err, start, YYCURSOR,    LEX_DIV_EQ}; }
    "++"             { return {err, start, YYCURSOR,    LEX_INC   }; }
    "--"             { return {err, start, YYCURSOR,    LEX_DEC   }; }
    "<="             { return {err, start, YYCURSOR,    LEX_LE    }; }
    ">="             { return {err, start, YYCURSOR,    LEX_GE    }; }
    "=="             { return {err, start, YYCURSOR,    LEX_EQUIV }; }
    "!="             { return {err, start, YYCURSOR,    LEX_NE    }; }

  */

  }

  return {err, start, YYCURSOR, EOF};
}

int re2c_lex(stream_t & strm, lexed_t & lx) 
{
  int err = 0;
  stream_pos_t pos;
  std::string ident;
  auto & buffer = strm.buffer;
  auto bufsz = buffer.size();
  auto bufbeg = &buffer[0];
  auto bufend = &buffer[bufsz];
  auto cur = bufbeg;
  
  while(cur < bufend) {

    int e, tok;
    char * tokstart;
    std::tie(e, tokstart, cur, tok) = scan(strm, cur);
    err += e;
    
    pos.begin = tokstart - bufbeg;
    pos.end = cur - bufbeg;

    if (tok == LEX_QUOTED) {
      pos.begin++;
      pos.end--;
    }
    
    auto len = pos.end - pos.begin;

    switch (tok) {
    #define TOKS_CASE(name, str, ...) \
      case name: lx.add(tok, pos, buffer.substr(pos.begin, len)); break;
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
