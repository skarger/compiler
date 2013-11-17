
#undef  DEBUG
#define DEBUG

#include "../include/parse-tree.h"
#include "../include/symbol.h"


struct traversal_data {
    SymbolTableContainer *stc;
    TypeNode *current_base_type;
};
typedef struct traversal_data traversal_data;


/* tree traversal */
void traverse_node(void *np, traversal_data *td);
void traverse_direct_abstract_declarator(void *np, traversal_data *td);
void traverse_conditional_statement(void *np, traversal_data *td);
void traverse_iterative_statement(void *np, traversal_data *td);
void traverse_pointers(void *np, traversal_data *td);

/* symbol creation during traversal */
TypeNode *create_base_type(Node *n);
