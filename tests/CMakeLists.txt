
target_sources( test_lex PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/test_hand.cpp )
target_sources( test_lex PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/test_fsm.cpp )

if (RE2C_EXECUTABLE)
  target_sources( test_lex PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/test_re2c.cpp )
endif()
