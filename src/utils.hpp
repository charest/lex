#ifndef CONTRA_STRING_UTILS_HPP
#define CONTRA_STRING_UTILS_HPP

#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

namespace lex {

//! count digits
size_t count_digits(int i);

//! string comparison
bool differs(const std::string & a, const std::string & b);

template<typename T = std::string>
void printLeft(std::ostream & os, int width, char sep, const T & val = std::string())
{
  os << std::left << std::setw(width) << std::setfill(sep) << val;
}

template<typename T = std::string>
void printRight(std::ostream & os, int width, char sep, const T & val = std::string())
{
  os << std::right << std::setw(width) << std::setfill(sep) << val;
}

std::string extract_to_newline(const std::string& input, size_t start);

std::vector<size_t> newline_positions(const std::string & text);

} // namespace

#endif // CONTRA_STRING_UTILS_HPP
