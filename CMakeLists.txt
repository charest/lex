cmake_minimum_required(VERSION 3.10)

project(LEX LANGUAGES CXX)

# cmake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
include(BuildType)

add_library(lex)
add_subdirectory(src)

add_executable(lexit)
add_subdirectory(app)
target_include_directories(lexit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(lexit PRIVATE lex)

#------------------------------------------------------------------------------#
# Enable Regression Tests
#------------------------------------------------------------------------------#
include(CTest) # note: this adds a BUILD_TESTING which defaults to ON

#------------------------------------------------------------------------------#
# Google test
#------------------------------------------------------------------------------#
if (BUILD_TESTING)

  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG "main"
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  
  include(GoogleTest)
  
  add_executable(test_lex)

  target_link_libraries(
    test_lex
    lex
    GTest::gtest_main
    GTest::gmock_main
  )

  target_compile_definitions(test_lex PUBLIC TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/")

  target_include_directories(test_lex PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
   
  add_subdirectory(tests)

  gtest_discover_tests(test_lex)

endif()


