
#include "parse-tree.h"

#ifndef SYMBOL_H
#define SYBMOL_H



enum scope_state {
    TOP_LEVEL,
    FUNCTION_DEF,
    FUNCTION_DEF_PARAMETERS,
    FUNCTION_BODY,
    FUNCTION_PROTOTYPE,
    FUNCTION_PROTO_PARAMETERS,
    BLOCK
};

#define TOP_LEVEL_SCOPE 0
#define FUNCTION_BODY_SCOPE 1

/* indicate whether we are starting or ending the traversal of a node */
#define START 1
#define END 2

/* for testing */
#define PASS 1
#define FAIL 2

/* overloading classes */
#define OTHER_NAMES 1
#define STATEMENT_LABELS 2



int get_state();
int get_scope();
int get_overloading_class();
void initialize_fsm();
void transition_scope(Node *n, int action);

#endif

