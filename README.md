compiler
========

Parser - construct a parse tree for a subset of the C language and print it.

October 23: Pretty printing works.
October 11: Pretty printing does not fully work. It has some parenthesization bugs.

Build:
make parser

Run:
./parser [input_file] [output_file]
Will use stdin and stdout if input_file or output_file ommitted.
Note: prints error messages to stderr.

Files:
Makefile
src/parser/parser.y : PS2 Bison file.
src/include/parser.h : PS2 header file
src/lexer/* : files for PS1 scanner. Required for parser.
src/utilities/* : utility functions, required to build but not specifically part of PS2.
test_input_valid : for testing parser $ ./parser test_input_invalid [output_file]
test_input_invalid : input that should provoke errors
test_input : the valid and invalid versions combined
test_output : output from testing parser
sencond_output : demonstrates that pretty printing is idempotent
typescript : transcript of building and testing parser