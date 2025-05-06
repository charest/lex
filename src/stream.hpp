#ifndef CONTRA_STREAM_HPP
#define CONTRA_STREAM_HPP

#include <istream>
#include <string>

namespace lex {

//==============================================================================
/// Stream position
//==============================================================================
struct stream_pos_t {
  std::ios::pos_type begin, end;
};


struct stream_t {

  std::istream & in;
  std::string name;

  stream_t(std::istream & s, const std::string & nm="")
    : in(s), name(nm) {}


};

} // namespace

#endif
