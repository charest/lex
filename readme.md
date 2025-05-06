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
