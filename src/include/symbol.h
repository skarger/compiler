
#include "parse-tree.h"

#ifndef SYMBOL_H
#define SYBMOL_H



enum scope_state {
    TOP_LEVEL,
    FUNCTION_DEF,
    FUNCTION_PARAMETERS,
    FUNCTION_BODY,
    FUNCTION_PROTOTYPE,
    BLOCK
};

#define FUNCTION_BODY_SCOPE 1

#define START 1
#define END 2

#define PASS 1
#define FAIL 2

struct traverse_data {
    int scope;
};










void scope_fsm_start(Node *n);
void scope_fsm_end(Node *n);
int get_state();
int get_scope();



int is_function_param(Node *);

char *get_scope_state_name(enum scope_state);

#endif

