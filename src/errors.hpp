#ifndef ERRORS_HPP
#define ERRORS_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace lex {

struct stream_t;
struct stream_pos_t;

/// dump out the current line
int error(stream_t & is, const std::string & msg);

/// dump out the current line
int error(
  stream_t & is,
  const std::string & msg,
  const stream_pos_t & pos,
  const std::vector<std::ios::pos_type> & lines);


} // namespace 

#endif // ERRORS_HPP
