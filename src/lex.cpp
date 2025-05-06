#include "errors.hpp"
#include "lex.hpp"
#include "stream.hpp"
#include "utils.hpp"

#include <cstdio>
#include <iostream>
#include <iomanip>

namespace lex {
  
/// Get an identifier string
std::string_view lexed_t::getIdentifierString(int i) const
{ 
  if (i<0 || i >= identifier_offsets.size()) return {};
  auto beg = i>0 ? identifier_offsets[i-1] : 0;
  auto len = identifier_offsets[i] - beg;
  return std::string_view(identifier_data).substr(beg, len);
}

/// Find an identifier from a token
int lexed_t::findIdentifier(int tok) const
{
  auto it = std::lower_bound(identifier_tokens.begin(), identifier_tokens.end(), tok);
  if (it != identifier_tokens.end() && (*it == tok))
    return std::distance(identifier_tokens.begin(), it);
  return -1;
}

/// Add the identifier string
void lexed_t::add(int token, stream_pos_t pos, const std::string & identifier)
{
    if (identifier.size()) {
      auto nidents = identifier_offsets.size();
      auto ntoks = tokens.size();
      // try to insert the identifier
      identifier_data += identifier;
      identifier_offsets.push_back(identifier_data.size());
      // add the token mapping
      identifier_tokens.push_back(ntoks);
    }
    tokens.push_back( token );
    token_pos.emplace_back( pos );
}


//==============================================================================
// Lexer output operator
//==============================================================================
void print(std::ostream& os, const lexed_t & res)
{
  auto n = res.tokens.size();
  int digits = count_digits(n);
  auto aw = std::max(digits+1, 7);
  auto bw = 6;
  auto cw = 14;
  auto dw = std::max(bw, 8);
  auto ew = 4*aw;

  printRight(os, aw, ' ', "TokenId");
  printRight(os, 2, ' ');
  printRight(os, bw, ' ', "TypeId");
  printRight(os, 2, ' ');
  printRight(os, cw, ' ', "TypeString");
  printRight(os, 2, ' ');
  printRight(os, dw, ' ', "IndentId");
  printRight(os, 2, ' ');
  printLeft (os, ew, ' ', "IdentString");
  os << std::endl;

  printRight(os, aw, '-');
  printRight(os, 2, ' ');
  printRight(os, bw, '-');
  printRight(os, 2, ' ');
  printRight(os, cw, '-');
  printRight(os, 2, ' ');
  printRight(os, dw, '-');
  printRight(os, 2, ' ');
  printLeft (os, ew, '-');
  os << std::endl;

  for (size_t i=0; i<n; ++i) {
    auto id = res.findIdentifier(i);
    auto tyid = res.tokens[i];
    auto tystr = lex_to_str(tyid);
    
    std::stringstream ss;
    if (id>=0)
      ss << "\"" << res.getIdentifierString(id) << "\"";

    printRight(os, aw, ' ', i);
    printRight(os, 2, ' ');
    printRight(os, bw, ' ', tyid);
    printRight(os, 2, ' ');
    printRight(os, cw, ' ', tystr);
    printRight(os, 2, ' ');
    if (id>=0)
      printRight(os, dw, ' ', id);
    else
      printRight(os, dw, ' ');
    printRight(os, 2, ' ');
    printLeft (os, ew, ' ', ss.str());
    os << std::endl;
  }

}
 
} // namespace
