#include <lex.hpp>

#include "test.hpp"
  
using namespace lex;
using testing::ElementsAre;

static void test(
  const std::string & inp,
  const std::vector<std::pair<int, std::string>> & ans,
  bool isBad=false)
{
  test_harness<fsm_lex>(inp, ans, isBad);
}

//ident 123
//1.23 1.2.3
//0 0120 0x120
//0 0120x12 1x14
//= == 
//+ += ++ - -= --

TEST(fsm, ident)
{
  test("ident", {{S_IDENT, "ident"}});
}

TEST(fsm, number)
{
  test("123", {{S_INT, "123"}});
  test("1.23", {{S_REAL, "1.23"}});
  test("0", {{S_INT, "0"}});
  test("0120", {{S_OCTAL, "0120"}});
  test("0x120", {{S_HEX, "0x120"}});
  test("0X120", {{S_HEX, "0X120"}});

  test("1.2.3", {{S_UNK, "1.2.3"}}, true);
  test("0120x12", {{S_UNK, "0120x12"}}, true);
  test("1x14", {{S_UNK, "1x14"}}, true);
}

TEST(fsm, symbol)
{
  test("=", {{'=', ""}});
  test("==", {{S_EQUIV, ""}});
  test("+", {{'+', ""}});
  test("+=", {{S_ADD_EQ, ""}});
  test("++", {{S_INC, ""}});
  test("-", {{'-', ""}});
  test("-=", {{S_SUB_EQ, ""}});
  test("--", {{S_DEC, ""}});
}
