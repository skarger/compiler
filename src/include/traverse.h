#ifndef TRAVERSE_H
#define TRAVERSE_H

#define DEBUG
#undef  DEBUG

#include <stdio.h>
#include "parse-tree.h"
#include "symbol.h"
#include "literal.h"

#define TRAVERSE


/* array bound related */
/* choosing these constants since they are invalid array bounds anyway */
#define UNSPECIFIED_VALUE   -2147483647
#define VARIABLE_VALUE      -2147483646
#define NON_INTEGRAL_VALUE  -2147483645
#define CAST_VALUE          -2147483644


struct TraversalData {
    SymbolTableContainer *stc;
    enum data_type current_base_type;
    FunctionParameter *current_param_list;
    Symbol *current_symbol;
    Symbol *dummy_prototype_parameter;
    enum Boolean processing_parameters;
    enum Boolean function_definition;
    enum Boolean function_prototype;
    FILE *outfile;
};
typedef struct TraversalData TraversalData;


/* tree traversal */
void start_traversal(Node *n);
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
void reset_current_symbol(TraversalData *td);
void record_current_symbol(TraversalData *td, Node *n);
void validate_symbol(Symbol *s, TraversalData *td);
void validate_function_symbol(Symbol *s, TraversalData *td);


#endif
