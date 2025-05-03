#include <hand.hpp>
#include <stream.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace lex;
using testing::ElementsAre;


//---------------------------------------------------------------------------
void test_w_ident(
  const std::string & inp,
  const std::vector<std::pair<int, std::string>> & ans,
  bool isBad=false)
{
  std::cout << "Testing: " << inp << std::endl;

  std::stringstream ss(inp);
  stream_t is(ss);
  lexed_t res;
  auto err = hand_lex(is, res);
  
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
 


//=============================================================================
// Individual tests
//=============================================================================

TEST(hand, ident) {
  test_w_ident("ident", {{LEX_IDENT, "ident"}});
  test_w_ident("id1ent", {{LEX_IDENT, "id1ent"}});
  test_w_ident("1ident", {{LEX_INT, "1"}, {LEX_IDENT, "ident"}});
}


TEST(hand, quote) {
  test_w_ident("\"Quoted\"", {{LEX_QUOTED, "Quoted"}});
}

TEST(hand, comment) {
  test_w_ident("# test", {{LEX_COMMENT, ""}});
  test_w_ident("# test\nident", {{LEX_COMMENT, ""}, {LEX_IDENT, "ident"}});
}

TEST(hand, number) {
  test_w_ident("1"      , LEX_INT);
  test_w_ident("12"     , LEX_INT);
  test_w_ident("1.2"    , LEX_REAL);
  test_w_ident(".2"     , LEX_REAL);
  test_w_ident("0.2"    , LEX_REAL);
  test_w_ident("1.2e5"  , LEX_REAL);
  test_w_ident("1.2E5"  , LEX_REAL);
  test_w_ident("1.2e-5" , LEX_REAL);
  test_w_ident("1.2e+5" , LEX_REAL);
  test_w_ident("1.2e+55", LEX_REAL);
  
  test_w_ident("1..2",  LEX_REAL, true);
  test_w_ident("1...2", LEX_REAL, true);
  test_w_ident("1.2e+", LEX_REAL, true);
  test_w_ident("1.2ee", LEX_REAL, true);
  test_w_ident("1.2e ", LEX_REAL, true);
}

TEST(hand, ops) {
  test("+" , '+');
  test("+=", LEX_ADD_EQ);
  test("-" , '-');
  test("-=", LEX_SUB_EQ);
  test("*" , '*');
  test("*=", LEX_MUL_EQ);
  test("=" , '=');
  test("==", LEX_EQUIV);
  test("!" , '!');
  test("!=", LEX_NE);
  test("<" , '<');
  test("<=", LEX_LE);
  test(">" , '>');
  test(">=", LEX_GE);
}

TEST(hand, punc) {
  test("," , ',');
  test(";" , ';');
  test("." , '.');
  test("%" , '%');
}

TEST(hand, function_add)
{
  std::stringstream ss;
  ss << "fn sum(i64 a, i64 b) return a+b";

  stream_t is(ss);
  lexed_t res;
  auto err = hand_lex(is, res);
  print(std::cout, res);
  EXPECT_THAT( res.tokens, ElementsAre(
    LEX_IDENT,
    LEX_IDENT,
    '(',
    LEX_IDENT,
    LEX_IDENT,
    ',',
    LEX_IDENT,
    LEX_IDENT,
    ')',
    LEX_IDENT,
    LEX_IDENT,
    '+',
    LEX_IDENT,
    LEX_EOF));
  ASSERT_FALSE(err);
  

  EXPECT_EQ(res.numIdentifiers(), 6);
  EXPECT_EQ(res.getIdentifierString(0), "fn");
  EXPECT_EQ(res.getIdentifierString(1), "sum");
  EXPECT_EQ(res.getIdentifierString(2), "i64");
  EXPECT_EQ(res.getIdentifierString(3), "a");
  EXPECT_EQ(res.getIdentifierString(4), "b");
  EXPECT_EQ(res.getIdentifierString(5), "return");
}

TEST(hand, error)
{
  std::stringstream ss;
  ss << "0..1";
 
  stream_t is(ss);
  lexed_t res;
  auto err = hand_lex(is, res);
  ASSERT_TRUE(err);
}
