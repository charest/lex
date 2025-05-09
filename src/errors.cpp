#include "stream.hpp"
#include "utils.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace lex {

//==============================================================================
/// Count the lines in the file
//==============================================================================
std::pair<size_t,size_t> count_lines(std::string & in, size_t pos)
{
  // seek to beginning of the file
  size_t lineCount=0;
  size_t lineStart = 0;

  // Count lines up to 
  char ch;
  for (; pos<in.size(); ++pos) {
    if (in[pos] == '\n') {
      lineCount++;
      lineStart = pos+1;
    }
  }

  return {lineCount, lineStart};
}

//==============================================================================
/// dump out the current line
//==============================================================================
int error(stream_t & is, const std::string & msg, std::size_t pos)
{
  // seek to beginning of the file
  auto [lineCount, lineStart] = count_lines(is.buffer, pos);

  // get the line with the error
  auto line = extract_to_newline(is.buffer, lineStart);
  
  // output
  if (is.name.size()) std::cerr << is.name << ":";
  auto col = pos - lineStart;
  std::cerr << lineCount+1 << ":" << col << ": error: " << msg << std::endl;
  std::cerr << line << std::endl;
  std::cerr << std::string(col-1, ' ') << "^" << std::endl;

  return 1;
}

//==============================================================================
/// dump out the current line
//==============================================================================
int error(
  stream_t & is,
  const std::string & msg,
  const stream_pos_t & pos)
{
  // figure out the line start
  size_t lineStart = 0;
  size_t lineNo = 0;
  auto & lines = is.newlines;

  if (lines.size()) {
    auto it = std::lower_bound(lines.begin(), lines.end(), pos.begin);
    lineStart = (it != lines.end()) ? *it : lines.back();
    lineNo = std::distance(lines.begin(), it);
  }

  // get the line
  auto line = extract_to_newline(is.buffer, lineStart);
  auto lineLen = line.size();

  // output
  if (is.name.size()) std::cerr << is.name << ":";
  auto colNo = pos.begin - lineStart;
  auto width = pos.end - pos.begin;

  std::cerr << lineNo+1 << ":" << colNo+1 << ": error: " << msg << std::endl;
  std::cerr << line << std::endl;
  std::cerr << std::string(colNo, ' ') << std::string(width, '^') << std::endl;

  return 1;
}

} // namespace
