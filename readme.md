# Lexer Comparison: FSM vs Hand-Written

This C++ project compares the performance and structure of two lexical analyzers:

- **FSM-based lexer**: Uses a finite state machine model to tokenize input.
- **Hand-written lexer**: Implements custom logic for tokenizing without an explicit FSM.

The goal is to analyze code maintainability, extensibility, and performance under various input scenarios.

---

## 🔧 Build Instructions

This project uses **CMake** for configuration and building.

### Prerequisites

- A C++ compiler with support for C++17 or later
- CMake 3.10+

### Build Steps

```bash
git clone https://github.com/charest/lex.git
cd lex
mkdir build && cd build
cmake ..
make
```

### Run tests
```bash
ctest
```

## 🏃 Run ```lexit```

### Generate Fake Syntax
```bash
usage: gen_random.py [-h] --output OUTPUT --lines LINES [--tok_per_line TOK_PER_LINE]

python3 tools/gen_random.py --output fake_program_10k.txt --lines 10000
```

### Run Lexical Analysis
```bash
  Usage: ./lexit <input_file> <lexer_type: fsm|hand|re2c> [--output <file>] [--iters 5]

 ./lexit ../tests/fake_program_10k.txt fsm
```
You should see output similar to the following:
```bash
Processing: fake_program_10k.txt
... Lexing via FSM ... 9.69757 ms
Avg Elapsed: 9.7356 ms
Tokens: 100000
Lines: 10000
```

## 🔍 Sample Results


[](bench.png)

### Cachegrind

Profile the lexer via Cachegrind to get cache simulations
```bash
valgrind --tool=cachegrind lexit ...
```

The results for 1M lines of simulated code are below: 

| Algorithm | I1 Miss | LLi Miss | D1 Miss | LLd Miss |
| ---- | ----- | ----- | ---- | ---- |
| Hand | 0.00% | 0.00% | 1.2% | 1.0% |
| FSM  | 0.00% | 0.00% | 0.9% | 0.7% |
| re2c | 0.00% | 0.00% | 1.4% | 1.1% |
