#include <lex.hpp>
#include <stream.hpp>
#include <utils.hpp>

#include <chrono>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
  
using namespace lex;
using testing::ElementsAre;

//---------------------------------------------------------------------------
static std::pair<lexed_t,int> test(const std::string & inp)
{
  std::stringstream ss(inp);
  auto is = make_stream(ss);
  lexed_t res;
  auto err = hand_lex(is, res);
  print(std::cout, res);

  return {res, err};
}

//---------------------------------------------------------------------------
static void test(
  const std::string & inp,
  const std::vector<std::pair<int, std::string>> & ans,
  bool isBad=false)
{
  using namespace lex;

  std::cout << "Testing: " << inp << std::endl;

  auto [res, err] = test(inp);
  
  auto nans = ans.size();
  EXPECT_EQ(res.numTokens(), nans);
  
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

//---------------------------------------------------------------------------
static void test_file(
  const std::string & inname,
  const std::string & gold = "",
  bool do_out = false)
{
  std::cout << "Processing: " << inname << std::endl;

  std::ifstream infile(inname);
  auto is = make_stream(infile, inname);

  auto start = std::chrono::high_resolution_clock::now();
  lexed_t res;
  auto err = hand_lex(is, res);
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> duration = end - start;

  std::cout << "Elapsed: " << duration.count() << " ms" << std::endl;
  std::cout << "Tokens: " << res.numTokens() << std::endl;
  std::cout << "Lines: " << is.newlines.size() << std::endl;
  
  if (gold.size()) {
  
    // output to gold file if requested
    if (do_out) {
      std::ofstream goldfile(gold);
      std::cout << "Writing To: " << gold << std::endl;
      print(goldfile, res);
    }
    // otherwise read it
    else {
      std::ifstream goldfile(gold);
      auto buf = std::istreambuf_iterator<char>(goldfile.rdbuf());
      std::string goldstr(buf, std::istreambuf_iterator<char>());
      std::ostringstream out;
      print(out, res);
      EXPECT_FALSE(differs(goldstr, out.str()));
    }

  } // gold
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
  test("# test\n", {{LEX_COMMENT, ""}});
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
  test("+" , {{'+',        ""}});
  test("+=", {{LEX_ADD_EQ, ""}});
  test("-" , {{'-',        ""}});
  test("-=", {{LEX_SUB_EQ, ""}});
  test("*" , {{'*',        ""}});
  test("*=", {{LEX_MUL_EQ, ""}});
  test("/" , {{'/',        ""}});
  test("/=", {{LEX_DIV_EQ, ""}});
  test("=" , {{'=',        ""}});
  test("==", {{LEX_EQUIV,  ""}});
  test("!" , {{'!',        ""}});
  test("!=", {{LEX_NE,     ""}});
  test("<" , {{'<',        ""}});
  test("<=", {{LEX_LE,     ""}});
  test(">" , {{'>',        ""}});
  test(">=", {{LEX_GE,     ""}});
}

TEST(hand, punc) {
  test("," , {{',', ""}});
  test(";" , {{';', ""}});
  test("." , {{'.', ""}});
  test("%" , {{'%', ""}});
}

TEST(hand, function_add)
{
  auto [res, err] = test("fn  sum(i64 a, i64 b) return a+b");
  
  ASSERT_FALSE(err);
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
    LEX_IDENT));
  

  EXPECT_EQ(res.numIdentifiers(), 9);
  EXPECT_EQ(res.getIdentifierString(0), "fn");
  EXPECT_EQ(res.getIdentifierString(1), "sum");
  EXPECT_EQ(res.getIdentifierString(2), "i64");
  EXPECT_EQ(res.getIdentifierString(3), "a");
  EXPECT_EQ(res.getIdentifierString(4), "i64");
  EXPECT_EQ(res.getIdentifierString(5), "b");
  EXPECT_EQ(res.getIdentifierString(6), "return");
  EXPECT_EQ(res.getIdentifierString(7), "a");
  EXPECT_EQ(res.getIdentifierString(8), "b");
}

TEST(hand, error)
{
  auto [res, err] = test("0..1");
  ASSERT_TRUE(err);
}

TEST(hand, fake_10k)
{
  test_file(
    TEST_DIR "fake_program_10k.txt",
    TEST_DIR "fake_program_10k.toks",
    false);
}
