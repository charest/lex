#include "stream.hpp"

#include <iostream>
#include <string>
#include <vector>

using pos_type = std::ios::pos_type;

namespace lex {

std::string get_line(std::istream & in, pos_type lineStart)
{
  in.seekg(lineStart);
  std::string line;
  std::getline(in, line);
  return line;
}

//==============================================================================
/// Count the lines in the file
//==============================================================================
std::pair<size_t,pos_type> count_lines(std::istream & in, pos_type pos)
{
  // seek to beginning of the file
  in.seekg(0);
  size_t lineCount=0;
  pos_type lineStart = 0;

  // Count lines up to 
  char ch;
  while (in.get(ch)) {
    if (ch == '\n') {
      lineCount++;
      lineStart = in.tellg();
    }
    if (in.tellg() >= pos) break;
  }

  return {lineCount, lineStart};
}

//==============================================================================
/// dump out the current line
//==============================================================================
int error(stream_t & is, const std::string & msg)
{
  auto & in = is.in;

  // get current position
  auto currentPos = in.tellg();

  // check if we reached the end of file
  auto is_eof = in.eof();
  in.clear(); // in case we reached end of stream

  if (is_eof) {
    in.seekg(0, std::ios::end);
    currentPos = in.tellg();
  }

  // seek to beginning of the file
  auto [lineCount, lineStart] = count_lines(in, currentPos);

  // get the line with the error
  auto line = get_line(in, lineStart);
  
  // output
  if (is.name.size()) std::cerr << is.name << ":";
  auto col = currentPos - lineStart + is_eof;
  std::cerr << lineCount+1 << ":" << col << ": error: " << msg << std::endl;
  std::cerr << line << std::endl;
  std::cerr << std::string(col-1, ' ') << "^" << std::endl;

  // go back to the original position
  in.seekg(currentPos);
  // seek to end if it was originally at the end
  if (is_eof) in.get();

  return 1;
}

//==============================================================================
/// dump out the current line
//==============================================================================
int error(
  stream_t & is,
  const std::string & msg,
  const stream_pos_t & pos,
  const std::vector<pos_type> & lines )
{
  auto & in = is.in;

  // get current position
  auto currentPos = in.tellg();

  // check if we reached the end of file
  auto is_eof = in.eof();
  in.clear(); // in case we reached end of stream

  // figure out the line start
  pos_type lineStart = lineStart = std::ios::beg;
  std::size_t lineNo = 0;

  if (lines.size()) {
    auto it = std::lower_bound(lines.begin(), lines.end(), pos.begin);
    lineStart = (it != lines.end()) ? *it : lines.back();
    lineNo = std::distance(lines.begin(), it);
  }

  // get the line
  auto line = get_line(in, lineStart);
  auto lineLen = line.size();

  // output
  if (is.name.size()) std::cerr << is.name << ":";
  auto colNo = pos.begin - lineStart;
  auto width = pos.end - pos.begin;

  std::cerr << lineNo+1 << ":" << colNo+1 << ": error: " << msg << std::endl;
  std::cerr << line << std::endl;
  std::cerr << std::string(colNo, ' ') << std::string(width, '^') << std::endl;

  // seek to end if it was originally at the end
  if (is_eof) {
    in.seekg(0, std::ios::end);
    in.get();
  }
  else {
    in.seekg(currentPos);
  }
  
  return 1;
}

} // namespace
