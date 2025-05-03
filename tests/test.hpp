#ifndef TEST_HPP
#define TEST_HPP

#include <stream.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

//---------------------------------------------------------------------------
template<auto LEX>
void test_harness(
  const std::string & inp,
  const std::vector<std::pair<int, std::string>> & ans,
  bool isBad=false)
{
  using namespace lex;

  std::cout << "Testing: " << inp << std::endl;

  std::stringstream ss(inp);
  stream_t is(ss);
  lexed_t res;
  auto err = LEX(is, res);
  
  auto nans = ans.size();
  EXPECT_EQ(res.numTokens(), nans+1);
  EXPECT_EQ(res.tokens.back(), LEX_EOF);
  
  for (int i=0; i<nans; ++i) {
    auto exp_tok = ans[i].first;
    auto & exp_id = ans[i].second;
    auto tok = res.tokens[i];
    EXPECT_EQ(tok, exp_tok);
    std::cout << "... [" << i << "] Expected: " << lex_to_str(tok);
    std::cout << " Got: " << lex_to_str(tok) << std::endl;
    auto id = res.findIdentifier(i);
    auto str = res.getIdentifierString(id);
    EXPECT_EQ(str, exp_id);
    std::cout << "... [" << i << "] Expected: " << exp_id;
    std::cout << " Got: " << str << std::endl;
  }

  if (isBad) ASSERT_TRUE(err);
  else       ASSERT_FALSE(err);
}
 

#endif // TEST_HPP
