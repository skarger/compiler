#ifndef TRAVERSE_H
#define TRAVERSE_H

#include "parse-tree.h"

/* define INTERACTIVE to receive output from parser for each entered top level decl */
/* if INTERACTIVE is not defined tree traversal will start upon reaching end-of-file */
#define INTERACTIVE
#undef INTERACTIVE

/* tree traversal */
void start_traversal(Node *n);

#endif
