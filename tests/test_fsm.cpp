#include <lex.hpp>

#include <stream.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
  
using namespace lex;
using testing::ElementsAre;

//---------------------------------------------------------------------------
static std::pair<lexed_t,int> test(const std::string & inp)
{
  auto table = make_fsm_table();

  std::stringstream ss(inp);
  stream_t is(ss);
  lexed_t res;
  auto err = fsm_lex(is, table, res);
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
static void test_file(const std::string & inname, bool do_out)
{
  std::cout << "Processing: " << inname << std::endl;
  
  auto table = make_fsm_table();


#define BUFF
#ifdef BUFF
  std::ifstream infile(inname);
  stream_t is(infile, inname);
#else
  std::ifstream infile(inname);
  auto inbuf = std::istreambuf_iterator<char>(infile.rdbuf());
  std::string instr(inbuf, std::istreambuf_iterator<char>());
  std::istringstream inss(instr);
  stream_t is(inss, inname);
#endif
  
  auto start = std::chrono::high_resolution_clock::now();
  lexed_t res;
  auto err = fsm_lex(is, table, res);
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> duration = end - start;

  std::cout << "Elapsed: " << duration.count() << " ms" << std::endl;
  std::cout << "Tokens: " << res.numTokens() << std::endl;
  std::cout << "Lines: " << res.line_start.size() << std::endl;
  
  std::string name = ::testing::UnitTest::GetInstance()->current_test_info()->name();
  std::string case_name = ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name();
  std::string outname = case_name + "." + name + ".txt";

  if (do_out) {
    std::cout << "Writing To: " << outname << std::endl;
    std::ofstream out(outname);
    print(out, res);
  }
}

//=============================================================================
// Individual tests
//=============================================================================


//ident 123
//1.23 1.2.3
//0 0120 0x120
//0 0120x12 1x14
//= == 
//+ += ++ - -= --

TEST(fsm, ident)
{
  test("ident", {{LEX_IDENT, "ident"}});
}

TEST(fsm, space)
{
  test("ident id", {{LEX_IDENT, "ident"}, {LEX_IDENT, "id"}});
  test("ident  id", {{LEX_IDENT, "ident"}, {LEX_IDENT, "id"}});
  test(" ident", {{LEX_IDENT, "ident"}});
}

TEST(fsm, line)
{
  test("ident\nid", {{LEX_IDENT, "ident"}, {LEX_IDENT, "id"}});
}

TEST(fsm, number)
{
  test("123",   {{LEX_INT,   "123"}});
  test("1.23",  {{LEX_REAL,  "1.23"}});
  test("0",     {{LEX_INT,   "0"}});
  test("0120",  {{LEX_OCTAL, "0120"}});
  test("0x120", {{LEX_HEX,   "0x120"}});
  test("0X120", {{LEX_HEX,   "0X120"}});

  test("1.2.3",   {{LEX_UNK, "1.2.3"}}, true);
  test("0120x12", {{LEX_UNK, "0120x12"}}, true);
  test("1x14",    {{LEX_UNK, "1x14"}}, true);
}

TEST(fsm, symbol)
{
  test("=",  {{'=',        ""}});
  test("==", {{LEX_EQUIV,  ""}});
  test("+",  {{'+',        ""}});
  test("+=", {{LEX_ADD_EQ, ""}});
  test("++", {{LEX_INC,    ""}});
  test("-",  {{'-',        ""}});
  test("-=", {{LEX_SUB_EQ, ""}});
  test("--", {{LEX_DEC,    ""}});
  test("*" , {{'*',        ""}});
  test("*=", {{LEX_MUL_EQ, ""}});
  test("/" , {{'/',        ""}});
  test("/=", {{LEX_DIV_EQ, ""}});
  test("!" , {{'!',        ""}});
  test("!=", {{LEX_NE,     ""}});
  //test("<" , {{'<',        ""}});
  //test("<=", {{LEX_LE,     ""}});
  //test(">" , {{'>',        ""}});
  //test(">=", {{LEX_GE,     ""}});
}

TEST(fsm, punc) {
  test("," , {{',', ""}});
  test(";" , {{';', ""}});
  test("." , {{'.', ""}});
  test("%" , {{'%', ""}});
}

TEST(fsm, function_add)
{
  auto [res, err] = test("fn  sum(i64 a, i64 b) return a+b");
  
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
  ASSERT_FALSE(err);
  

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

TEST(fsm, lines)
{
  auto [res, err] = test("0\n1");
  ASSERT_EQ( res.line_start.size(), 1);
  EXPECT_THAT( res.line_start, ElementsAre(2) );
  ASSERT_FALSE(err);
}

TEST(fsm, fake_10k)
{
  test_file(TEST_DIR "fake_program_10k.txt", true);
}

#define BIGFILES
#ifdef BIGFILES
TEST(fsm, fake_100k)
{
  test_file(TEST_DIR "fake_program_100k.txt", true);
}

TEST(fsm, fake_1m)
{
  test_file(TEST_DIR "fake_program_1m.txt", false);
}
#endif
