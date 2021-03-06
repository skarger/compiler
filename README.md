compiler
========

Tested on Linux (Ubuntu 8.04) and Mac OS X (10.8.5); untested on Windows.

Each "main" program will use stdin/stdout if input_file or output_file omitted.
Note: prints error messages to stderr.

### Lexer
Scan a C input file and output information about each distinct token.
```
# Build:
make lexer-main
# Run:
./lexer-main [input_file] [output_file]
```


### Parser
Construct a parse tree for a subset of the C language and print it.
```
# Build:
make parser-main
# Run:
./parser-main [input_file] [output_file]
# Test:
make test-parser-output
make test-parser-errors
```


### Symbol Table
Traverse a parse tree and create symbol tables for each identifier, for both declarations and references.
```
# Build:
make symbol-main
# Run:
./symbol-main [input_file] [output_file]
# Test:
make test-symbol # (redirects error messages to /dev/null)
make test-symtab-errors # (shows error messages emitted by program)
```


### IR Generator
Output Intermediate Representation instructions given
a parse tree and symbol table derived from a C source file.
```
# Build:
make ir-main
# Run:
./ir-main [input_file] [output_file]
# Test:
make test-ir
```


### MIPS Assembly Generator
Output MIPS assembly code, fit for running in [SPIM](http://pages.cs.wisc.edu/~larus/spim.html) or even a real MIPS machine!
```
# Build:
make mips-main
# Run:
./mips-main [input_file] [output_file]
# Test:
make test-mips
```


### Files:
./src: Source files for compiler components.

./src/include: header files

./test: Testing scripts.

gtest-1.7.0 is [Google Test](https://code.google.com/p/googletest/), which is the only third-party library.
