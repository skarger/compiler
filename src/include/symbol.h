
#include "parse-tree.h"

#ifndef SYMBOL_H
#define SYBMOL_H



enum scope_state {
    TOP_LEVEL,
    FUNCTION_DEFINTION,
    FUNCTION_PARAMETERS,
    FUNCTION_BODY,
    FUNCTION_PROTOTYPE,
    BLOCK
};

struct traverse_data {
    int scope;
};










void scope_fsm_start(Node *n);
/*
void scope_fsm_end(Node *n);
*/

int get_scope();

void new_scope();

void previous_scope();

int is_function_param(Node *);

#endif

