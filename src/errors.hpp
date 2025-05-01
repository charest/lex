#ifndef ERRORS_HPP
#define ERRORS_HPP

#include <iostream>
#include <sstream>
#include <string>

namespace lex {

struct stream_t;
struct stream_pos_t;

/// dump out the current line
int error(stream_t & is, const std::string & msg);
int error(
  stream_t & is,
  const std::string & msg,
  stream_pos_t pos);

} // namespace 

#endif // ERRORS_HPP
