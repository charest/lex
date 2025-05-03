
#include <gtest/gtest.h>
#include <gmock/gmock.h>

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
}

TEST(fsm, symbol)
{
  test("=", {{'=', ""}});
}