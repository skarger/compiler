#ifndef TRAVERSE_H
#define TRAVERSE_H

#include "parse-tree.h"

/* define PRETTY_PRINT for parser tests */
#define PRETTY_PRINT
#undef PRETTY_PRINT

#define COLLECT_SYMBOLS
#undef COLLECT_SYMBOLS

#define COMPUTE_IR
#undef COMPUTE_IR

#define PRINT_IR
#undef PRINT_IR

#define MIPS_ASM
#undef MIPS_ASM


/* define INTERACTIVE to receive output from parser for each entered top level decl */
/* if INTERACTIVE is not defined tree traversal will start upon reaching end-of-file */
#define INTERACTIVE
#undef INTERACTIVE

/* tree traversal */
void start_traversal(Node *n);

#endif
