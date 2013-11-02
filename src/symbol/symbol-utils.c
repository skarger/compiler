#include <stdio.h>
#include "../include/uthash.h"
#include "../include/symbol.h"
#include "../include/token.h"
#include "../include/parse-tree.h"


/*
 * define a finite state machine to help with scope determination
 * while traversing parse tree.
 */

/*

* states
TOP_LEVEL
FUNCTION_DEF
FUNCTION_PARAMETERS
FUNCTION_BODY
FUNCTION_PROTOTYPE
BLOCK

* overloading classes
OTHER_NAME
STATEMENT_LABEL

* transitions
* Functions may be declared only at file scope.
TOP_LEVEL -> FUNCTION_DEF
    node: FUNCTION_DEFINITION
FUNCTION_DEF -> FUNCTION_PARAMETERS
    node: (PARAMETER_LIST | PARAMETER_DECL | TYPE_SPECIFIER == VOID)
FUNCTION_PARAMETERS -> FUNCTION_BODY
    node: start of COMPOUND_STATEMENT
FUNCTION_BODY -> TOP_LEVEL
    node: end of COMPOUND_STATEMENT

TOP_LEVEL -> FUNCTION_PROTOTYPE
    node: FUNCTION_DECLARATOR
FUNCTION_PROTOTYPE -> FUNCTION_PARAMETERS
    node: (PARAMETER_LIST | PARAMETER_DECL | TYPE_SPECIFIER == VOID)
FUNCTION_PARAMETERS -> TOP_LEVEL
    node: end of FUNCTION_DECLARATOR

FUNCTION_BODY -> BLOCK
    node: start of COMPOUND_STATEMENT
BLOCK -> FUNCTION_BODY
    node: end of COMPOUND_STATEMENT

BLOCK -> BLOCK
    node: start of COMPOUND_STATEMENT
BLOCK -> BLOCK
    node: end of COMPOUND_STATEMENT
*/

static int current_state = TOP_LEVEL;
static int scope = TOP_LEVEL;

static void set_state(int state) {
    current_state = state;
}

static void new_scope() {
    scope++;
}

static void previous_scope() {
    scope--;
}


int get_state() {
    return current_state;
}

int get_scope() {
    return scope;
}


int main() {
    enum scope_state cur = get_state();
    printf("current state: %s scope: %d\n", get_scope_state_name(cur), get_scope());

    Node *n = malloc(sizeof(Node));

    printf("function definition:\n");
    test_transition(n, FUNCTION_DEFINITION, START, FUNCTION_DEF, 0);
    test_transition(n, PARAMETER_LIST, START, FUNCTION_PARAMETERS, 1);
    test_transition(n, COMPOUND_STATEMENT, START, FUNCTION_BODY, 1);
    test_transition(n, COMPOUND_STATEMENT, START, BLOCK, 2);
    test_transition(n, COMPOUND_STATEMENT, START, BLOCK, 3);
    test_transition(n, COMPOUND_STATEMENT, END, BLOCK, 2);
    test_transition(n, COMPOUND_STATEMENT, END, FUNCTION_BODY, 1);
    test_transition(n, COMPOUND_STATEMENT, END, TOP_LEVEL, 0);


    printf("function prototype:\n");
    set_state(TOP_LEVEL);
    n->n_type = TOP_LEVEL;
    test_transition(n, FUNCTION_DECLARATOR, START, FUNCTION_PROTOTYPE, 0);
    test_transition(n, PARAMETER_DECL, START, FUNCTION_PARAMETERS, 1);
    test_transition(n, FUNCTION_DECLARATOR, END, TOP_LEVEL, 0);

    return 0;
}

void transition_scope(Node *n, int action) {
    if (action == START) {
        scope_fsm_start(n);
    } else {
        scope_fsm_end(n);
    }
}


void scope_fsm_start(Node *n) {
    enum node_type nt = n->n_type;
    if (nt == FUNCTION_DEFINITION && get_state() == TOP_LEVEL) {
        set_state(FUNCTION_DEF);
    } else if (is_function_param(n) && get_state() == FUNCTION_DEF) {
        new_scope();
        set_state(FUNCTION_PARAMETERS);
    } else if (nt == COMPOUND_STATEMENT && get_state() == FUNCTION_PARAMETERS) {
        set_state(FUNCTION_BODY);
        /* already entered the new scope for FUNCTION_PARAMETERS */
    } else if (nt == FUNCTION_DECLARATOR && get_state() == TOP_LEVEL) {
        set_state(FUNCTION_PROTOTYPE);
    } else if (is_function_param(n) && get_state() == FUNCTION_PROTOTYPE) {
        new_scope();
        set_state(FUNCTION_PARAMETERS);
    } else if (nt == COMPOUND_STATEMENT && get_state() == FUNCTION_BODY) {
        new_scope();
        set_state(BLOCK);
    } else if (nt == COMPOUND_STATEMENT && get_state() == BLOCK) {
        new_scope();
        set_state(BLOCK);
    }
}


void scope_fsm_end(Node *n) {
    enum node_type nt = n->n_type;

    if (nt == COMPOUND_STATEMENT && get_state() == FUNCTION_BODY) {
        /* end of function definition */
        previous_scope();
        set_state(TOP_LEVEL);
    } else if (FUNCTION_DECLARATOR && get_state() == FUNCTION_PARAMETERS) {
        /* end of function prototype */
        previous_scope();
        set_state(TOP_LEVEL);
    } else if (nt == COMPOUND_STATEMENT && get_state() == BLOCK) {
        previous_scope();
        if (scope <= FUNCTION_BODY_SCOPE) {
            set_state(FUNCTION_BODY);
        } else {
            set_state(BLOCK);
        }
    }
}

int is_function_param(Node *n) {
    return (n->n_type == PARAMETER_LIST ||  n->n_type == PARAMETER_DECL ||
           (n->n_type == TYPE_SPECIFIER && n->data.symbols[TYPE_SPEC] == VOID));
}


/*
 * get_scope_state_name
 *   Get the name of a state discovered by symbol table generator
 *
 * Parameters:
 *   ss - enum scope_state - the scope state returned by finite state machine
 *
 * Return: A pointer to the zero-terminated name.
 * Side effects: none
 *
 */
char *get_scope_state_name(enum scope_state ss) {
    switch (ss) {
    #define CASE_FOR(ss) case ss: return #ss
        CASE_FOR(TOP_LEVEL);
        CASE_FOR(FUNCTION_DEF);
        CASE_FOR(FUNCTION_PARAMETERS);
        CASE_FOR(FUNCTION_BODY);
        CASE_FOR(FUNCTION_PROTOTYPE);
        CASE_FOR(BLOCK);
    #undef CASE_FOR
        default: return "";
  }
}


