#include <lex.hpp>
#include <stream.hpp>

#include <chrono>
#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#define FOR_LEXERS(DO) \
  DO(HAND, "hand") \
  DO(FSM,  "fsm")

enum Opts {
#define LEXER_TOK(name, str) name,
FOR_LEXERS(LEXER_TOK)
#undef LEXER_TOK
};

void print_usage(char* argv[]) {
  std::cerr << "Usage: " << argv[0] << " <input_file> <lexer_type: fsm|hand> ";
  std::cerr << "[--output <file>] [--buffered]\n";
}

bool valid_lexer(const std::string & ty)
{
  #define LEXER_TEST(name, str) if (ty == str) return true;
  FOR_LEXERS(LEXER_TEST)
  #undef LEXER_TEST
  return false;
}

using namespace lex;

int main(int argc, char* argv[]) {

  // check arg count and print usage if necessary
  if (argc < 3) {
    print_usage(argv);
    return 1;
  }

  // required parameters
  std::string filename = argv[1];
  std::string lexer_type = argv[2];

  if (!valid_lexer(lexer_type)) {
    std::cout << "Invalid lexer type '" << lexer_type << "'" << std::endl;
    print_usage(argv);
    return 1;
  }

  // Parse optional args
  std::string output_file;
  bool  buffered_io= false;

  for (int i = 3; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--output" && i + 1 < argc) {
      output_file = argv[++i];
    } else if (arg == "--buffered") {
      buffered_io = true;
    } else if (arg == "--help" ) {
      print_usage(argv);
      return 0;
    } else {
      std::cerr << "Unknown option: " << arg << '\n';
      print_usage(argv);
      return 1;
    }
  }

  // Get IO
  std::cout << "Processing: " << filename << std::endl;

  std::ifstream infile(filename);
  std::unique_ptr<stream_t> is;
  std::string bufstr;
  std::istringstream bufss;

  if (buffered_io) {
    std::cout << "Buffering input." << std::endl;
    auto inbuf = std::istreambuf_iterator<char>(infile.rdbuf());
    bufstr = std::string(inbuf, std::istreambuf_iterator<char>());
    bufss = std::istringstream(bufstr);
    is = std::make_unique<stream_t>(bufss, filename);
  }
  else{
    is = std::make_unique<stream_t>(infile, filename);
  }

  // Process
  auto start = std::chrono::high_resolution_clock::now();
  lexed_t res;
  int err = 0;
  if (lexer_type == "hand") {
    std::cout << "... Lexing via hand lexer ..." << std::endl;
    err = hand_lex(*is, res);
  }
  else if (lexer_type == "fsm" ) {
    std::cout << "... Lexing via FSM ..." << std::endl;
    auto table = make_fsm_table();
    err = fsm_lex(*is, table, res);
  }
  else {
    std::cerr << "Unknown lexer type: '" << lexer_type << "'" << std::endl;
    return -1;
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> duration = end - start;

  std::cout << "Elapsed: " << duration.count() << " ms" << std::endl;
  std::cout << "Tokens: " << res.numTokens() << std::endl;
  std::cout << "Lines: " << res.line_start.size() << std::endl;
  
  // output
  if (output_file.size()) {
    std::cout << "Writing To: " << output_file << std::endl;
    std::ofstream out(output_file);
    print(out, res);
  }

  return err;
}
