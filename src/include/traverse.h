
#define DEBUG

#include "../include/parse-tree.h"
#include "../include/symbol.h"


struct traverse_data_ST {
    SymbolTableContainer *stc;
};
typedef struct traverse_data_ST traverse_data_ST;


/* tree traversal */
void traverse_node(void *np, traverse_data_ST *data_st);
void traverse_direct_abstract_declarator(void *np, traverse_data_ST *data_st);
void traverse_conditional_statement(void *np, traverse_data_ST *data_st);
void traverse_iterative_statement(void *np, traverse_data_ST *data_st);
void traverse_pointers(void *np, traverse_data_ST *data_st);
