#ifndef SYMBOL_TRAVERSAL_H
#define SYMBOL_TRAVERSAL_H

#include <stdio.h>
#include "parse-tree.h"
#include "symbol.h"
#include "literal.h"

/* array bound related */
/* choosing these constants since they are invalid array bounds anyway */
#define UNSPECIFIED_VALUE   -2147483647
#define VARIABLE_VALUE      -2147483646
#define NON_INTEGRAL_VALUE  -2147483645
#define CAST_VALUE          -2147483644

struct SymbolCreationData {
    SymbolTableContainer *stc;
    enum data_type current_base_type;
    FunctionParameter *current_param_list;
    Symbol *current_symbol;
    Symbol *dummy_symbol;
    Boolean processing_parameters;
    Boolean function_def_spec;
    Boolean function_prototype;
    FILE *outfile;
};
typedef struct SymbolCreationData SymbolCreationData;


/* tree traversal */
void traverse_node(Node *n, SymbolCreationData *scd);
void traverse_direct_abstract_declarator(Node *n, SymbolCreationData *scd);
void traverse_conditional_statement(Node *n, SymbolCreationData *scd);
void traverse_iterative_statement(Node *n, SymbolCreationData *scd);
void traverse_pointers(Node *np, SymbolCreationData *scd);
unsigned long resolve_constant_expr(Node *n);

void print_symbol(FILE *out, Symbol *s);
void print_symbol_table(FILE *out, SymbolTable *st);
void print_symbol_param_list(FILE *out, Symbol *s);

/* symbol creation during traversal */
void create_symbol_if_necessary(SymbolCreationData *scd);
void reset_current_symbol(SymbolCreationData *scd);
void record_current_symbol(SymbolCreationData *scd, Node *n);
void validate_symbol(Symbol *s, SymbolCreationData *scd);
void validate_function_symbol(Symbol *s, SymbolCreationData *scd);
void validate_statement_labels(SymbolCreationData *scd);


#endif
