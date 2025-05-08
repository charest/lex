
target_sources( lex PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/errors.cpp )
target_sources( lex PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/lex.cpp )
target_sources( lex PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/hand.cpp )
target_sources( lex PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/fsm.cpp )
target_sources( lex PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp )

if (RE2C_EXECUTABLE)
  # Input and output files
  set(RE2C_INPUT  ${CMAKE_CURRENT_SOURCE_DIR}/re2c.re)
  set(RE2C_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/re2c.cpp)

  # Create custom command to generate lexer.c from lexer.re
  add_custom_command(
    OUTPUT ${RE2C_OUTPUT}
    COMMAND ${RE2C_EXECUTABLE} -o ${RE2C_OUTPUT} ${RE2C_INPUT}
    DEPENDS ${RE2C_INPUT}
    COMMENT "Generating C source from ${RE2C_INPUT} with re2c"
    VERBATIM
  )

  # Custom target to drive the generation
  add_custom_target(generate_re2c DEPENDS ${RE2C_OUTPUT})

  # Ensure target depends on the generation
  add_dependencies(lex generate_re2c)

  # Prevent configure-time check for file existence
  set_source_files_properties(${RE2C_OUTPUT} PROPERTIES GENERATED TRUE)

  # Add the generated source to library
  target_sources( lex PRIVATE  ${RE2C_OUTPUT} )

endif()
