#ifndef TRAVERSE_H
#define TRAVERSE_H

#define DEBUG
#undef  DEBUG

#include <stdio.h>
#include "../include/parse-tree.h"
#include "../include/symbol.h"

#define TRAVERSE


struct TraversalData {
    SymbolTableContainer *stc;
    enum data_type current_base_type;
    Symbol *current_symbol;
    enum Boolean processing_parameter_decl;
    FILE *outfile;
};
typedef struct TraversalData TraversalData;


/* tree traversal */
void traverse_node(Node *n, TraversalData *td);
void traverse_direct_abstract_declarator(void *np, TraversalData *td);
void traverse_conditional_statement(void *np, TraversalData *td);
void traverse_iterative_statement(void *np, TraversalData *td);
void traverse_pointers(Node *np, TraversalData *td);
unsigned long resolve_constant_expr(Node *n);

void print_symbol(FILE *out, Symbol *s);
void print_symbol_table(FILE *out, SymbolTable *st);
void print_symbol_param_list(FILE *out, Symbol *s);

/* symbol creation during traversal */
void create_symbol_if_necessary(TraversalData *td);
void reset_current_symbol();


#endif
