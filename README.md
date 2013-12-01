compiler
========

Symbol Table - traverse a parse tree and create symbol tables for each
identifier, for both declarations and references.

Build:
make symbol-main

Run:
./symbol-main [input_file] [output_file]
Will use stdin and stdout if input_file or output_file ommitted.
Pretty prints the parse tree with comments for each symbol.
Note: prints error messages to stderr.

Test:
make test-symbol        # redirects error messages to /dev/null
make test-symtab-errors # shows error messages emitted by program


Parser - construct a parse tree for a subset of the C language and print it.

Build:
make parser-main

Run:
./parser-main [input_file] [output_file]
Will use stdin and stdout if input_file or output_file ommitted.
Note: prints error messages to stderr.

Test:
make test-parser-output
make test-parser-errors


Files:
Makefile
./src: Source files for compiler components.
include  lexer	parser	symbol	utilities

./src/include: header files

./src/symbol:
symbol-main.c: executable program for symbol table generation. Calls yyparse.
traverse.c: Parse tree traversal procedures that create symbols.
symbol-utils.c: Utility functions for symbols and symbol tables.
scope-fsm.c: Finite state machine to manage scope and overloading class.

./src/parser:
parser-main.c: main program for parser
parser.y: Bison grammar specification and parse tree building helpers
