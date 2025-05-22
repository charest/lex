import argparse
import random
import string

def generate_fake_program(output_file, num_lines, tokens_per_line):
  
  # Sample token pools (mimicking programming syntax)
  keywords = ['if', 'else', 'for', 'while', 'return', 'def', 'class', 'import', 'from', 'try', 'except', 'with', 'as']
  operators = ['+', '-', '*', '/', '%', '=', '==', '!=', '<', '>', '<=', '>=', 'and', 'or', 'not']
  symbols = ['(', ')', '{', '}', '[', ']', ':', ',', '.', ';']
  identifiers = ['var' + str(i) for i in range(1000)]
  ints = [str(random.randint(0, 10000)) for _ in range(1000)]
  reals = [str(random.uniform(0, 10000)) for _ in range(1000)]
  
  TOKEN_TYPES = {
    'keyword': (keywords, 1),
    'operator': (operators, 1),
    'symbol': (symbols, 1),
    'ints': (ints, 1),
    'reals': (reals, 1),
    'identifier': (identifiers, 1)
  }

  def choose_weighted_token():
    weighted_pool = []
    for name, (tokens, weight) in TOKEN_TYPES.items():
        weighted_pool.extend([tokens] * weight)
    pool = random.choice(weighted_pool)
    return random.choice(pool)
  
  # Function to generate a random line of tokens
  def generate_random_line():
      return ' '.join(choose_weighted_token() for _ in range(tokens_per_line)) + '\n'
  
  # Write the fake program to file
  if output_file:
    with open(output_file, 'w') as f:
        for _ in range(num_lines):
            f.write(generate_random_line()) 
    print(f"File '{output_file}' created with {num_lines} lines.")
  else:
      for _ in range(num_lines):
          print(generate_random_line(), end="")

def main():
    parser = argparse.ArgumentParser(description="Generate a large fake code file with random tokens.")
    parser.add_argument("--output", required=True, help="Path to the output file (e.g., fake_program.txt)")
    parser.add_argument("--lines", required=True, type=int, help="Number of lines to generate")
    parser.add_argument("--tok_per_line", type=int, help="Number of tokens per line", default=10)

    args = parser.parse_args()
    generate_fake_program(args.output, args.lines, args.tok_per_line)

if __name__ == "__main__":
    main()

