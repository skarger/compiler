#include "literal.h"

#ifndef SCOPE_FSM_H
#define SCOPE_FSM_H

/* give names to fundamental scope levels. deeper levels need no name */
#define TOP_LEVEL_SCOPE 0
#define FUNCTION_SCOPE 1

/* indicator for whether we are starting or ending the traversal of a node */
#define START 1
#define END 2


enum scope_state {
    TOP_LEVEL,
    FUNC_DEF,
    FUNC_DEF_DECL,
    FUNC_DEF_PARAMS,
    FUNC_BODY,
    BLOCK
};

/* finite state machine functions */
void transition_scope(Node *n, int action, SymbolTableContainer *stc);
Boolean is_inner_block(int scope);

/* TODO: these should be static methods. they are only visible for testing */
void initialize_fsm();
int get_state();
int get_scope();
int get_overloading_class();
char *get_scope_state_name(enum scope_state);
char *get_overloading_class_name(int oc);


#endif
