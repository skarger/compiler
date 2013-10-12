compiler
========

Parser

Build:
make parser

Run:
./parser [input_file] [output_file]
Will use stdin and stdout if input_file or output_file ommitted.
Note: prints error messages to stderr.

Files:
Makefile
test_input : for testing parser: $ ./parser test_input
src/parser/parser.y : PS2 Bison file.
src/include/parser.h : PS2 header file
src/lexer/* : files for PS1 scanner. Required for parser.
src/utilities/* : utility functions, required to build but not specifically part of PS2.

