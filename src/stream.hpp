#ifndef CONTRA_STREAM_HPP
#define CONTRA_STREAM_HPP

#include <istream>
#include <string>
#include <vector>

namespace lex {

struct stream_pos_t {
  std::size_t begin, end;
};


struct stream_t {

  std::string buffer;
  std::string name;
  std::vector<size_t> newlines;

};

stream_t make_stream(std::istream & in, const std::string & name = "");

} // namespace

#endif
