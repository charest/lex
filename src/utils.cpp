#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

namespace lex {

////////////////////////////////////////////////////////////////////////////////
//! count digits
////////////////////////////////////////////////////////////////////////////////
size_t count_digits(int i)
{
  int count = 0;
  while (i != 0) {
    i /= 10;
    count++;
  }
  return count;
}

////////////////////////////////////////////////////////////////////////////////
//! string comparison
////////////////////////////////////////////////////////////////////////////////
bool differs(const std::string & a, const std::string & b)
{
  auto na = a.size();
  if (na != b.size()) return true;

  for (size_t i=0; i<na; ++i)
    if (a[i] != b[i]) return true;
  
  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::string extract_to_newline(const std::string& input, size_t start) {
  size_t end = input.find('\n', start);
  if (end == std::string::npos) {
    // No newline found, extract to end of string
    return input.substr(start);
  }
  return input.substr(start, end - start);
}

////////////////////////////////////////////////////////////////////////////////
std::vector<size_t> newline_positions(const std::string & text)
{
  std::vector<size_t> newlines;
  auto sz = text.size();

  for (size_t i=0; i<sz; ++i)
    if (text[i] == '\n')
      newlines.push_back(i);

  return newlines;
}

} // namespace
