#include <lex.hpp>

#include "test.hpp"
  
using namespace lex;
using testing::ElementsAre;

static void test(
  const std::string & inp,
  const std::vector<std::pair<int, std::string>> & ans,
  bool isBad=false)
{
  test_harness<hand_lex>(inp, ans, isBad);
}

//=============================================================================
// Individual tests
//=============================================================================

TEST(hand, ident) {
  test("ident", {{LEX_IDENT, "ident"}});
  test("id1ent", {{LEX_IDENT, "id1ent"}});
  test("1ident", {{LEX_INT, "1"}, {LEX_IDENT, "ident"}});
}


TEST(hand, quote) {
  test("\"Quoted\"", {{LEX_QUOTED, "Quoted"}});
}

TEST(hand, comment) {
  test("# test", {{LEX_COMMENT, ""}});
  test("# test\nident", {{LEX_COMMENT, ""}, {LEX_IDENT, "ident"}});
}

TEST(hand, number) {
  test("1"      , {{LEX_INT, "1"}});
  test("12"     , {{LEX_INT, "12"}});
  test("1.2"    , {{LEX_REAL, "1.2"}});
  test(".2"     , {{LEX_REAL, ".2"}});
  test("0.2"    , {{LEX_REAL, "0.2"}});
  test("1.2e5"  , {{LEX_REAL, "1.2e5"}});
  test("1.2E5"  , {{LEX_REAL, "1.2E5"}});
  test("1.2e-5" , {{LEX_REAL, "1.2e-5"}});
  test("1.2e+5" , {{LEX_REAL, "1.2e+5"}});
  test("1.2e+55", {{LEX_REAL, "1.2e+55"}});
  
  test("1..2",  {{LEX_REAL, "1..2"}}, true);
  test("1...2", {{LEX_REAL, "1...2"}}, true);
  test("1.2e+", {{LEX_REAL, "1.2e+"}}, true);
  test("1.2ee", {{LEX_REAL, "1.2ee"}}, true);
  test("1.2e ", {{LEX_REAL, "1.2e "}}, true);
}

TEST(hand, ops) {
  test("+" , {{'+', ""}});
  test("+=", {{LEX_ADD_EQ, ""}});
  test("-" , {{'-', ""}});
  test("-=", {{LEX_SUB_EQ, ""}});
  test("*" , {{'*', ""}});
  test("*=", {{LEX_MUL_EQ, ""}});
  test("=" , {{'=', ""}});
  test("==", {{LEX_EQUIV, ""}});
  test("!" , {{'!', ""}});
  test("!=", {{LEX_NE, ""}});
  test("<" , {{'<', ""}});
  test("<=", {{LEX_LE, ""}});
  test(">" , {{'>', ""}});
  test(">=", {{LEX_GE, ""}});
}

TEST(hand, punc) {
  test("," , {{',', ""}});
  test(";" , {{';', ""}});
  test("." , {{'.', ""}});
  test("%" , {{'%', ""}});
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
