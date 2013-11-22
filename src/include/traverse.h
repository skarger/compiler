
#undef  DEBUG
#define DEBUG

#include <stdio.h>
#include "../include/parse-tree.h"
#include "../include/symbol.h"


struct traversal_data {
    SymbolTableContainer *stc;
    enum data_type current_base_type;
    Symbol *current_symbol;
    FILE *outfile;
};
typedef struct traversal_data traversal_data;


/* tree traversal */
void traverse_node(Node *n, traversal_data *td);
void traverse_direct_abstract_declarator(void *np, traversal_data *td);
void traverse_conditional_statement(void *np, traversal_data *td);
void traverse_iterative_statement(void *np, traversal_data *td);
void traverse_pointers(Node *np, traversal_data *td);
void print_symbol(FILE *out, Symbol *s);
void print_symbol_table(FILE *out, SymbolTable *st);

/* symbol creation during traversal */
void create_symbol_if_necessary(traversal_data *td);
void reset_current_symbol();
