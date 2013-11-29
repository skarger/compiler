#include "../include/parse-tree.h"
#include "../../y.tab.h"
#include "../include/symbol-utils.h"
#include "../include/scope-fsm.h"

/*
 * define a finite state machine to help with scope determination
 * while traversing parse tree.
 */
static enum scope_state current_state = TOP_LEVEL;
static int scope = TOP_LEVEL;
static int overloading_class = OTHER_NAMES;
static enum Boolean fsm_initialized = FALSE;

static void set_state(int state);
static void set_scope(int s);
static void set_overloading_class(int oc);
static void new_scope();
static void previous_scope();

static enum Boolean fsm_is_ready();
static void scope_fsm_start(Node *n);
static void scope_fsm_end(Node *n);
static int node_is_function_param(Node *);
static int node_begins_statement_label(Node *n);

#ifndef SYMBOL_TEST
static void initialize_fsm();
static int get_state();
static int get_scope();
static int get_overloading_class();
static char *get_scope_state_name(enum scope_state);
static char *get_overloading_class_name(int oc);
#endif

/*
 * transition_scope
 *      update program scope in response to the given node
 * parameters:
 *  n - pointer to the Node prompting the call to this function
 *  action - START or END, depending on what point of the parse tree traversal
 *                         the caller is one with the given node
 *  stc - pointer to SymbolTableContainer that will be updated with the
 *        new scope and overloading class
 * returns: none
 * side effects: none
 */
void transition_scope(Node *n, int action, SymbolTableContainer *stc) {
    if (!fsm_is_ready()) {
        initialize_fsm();
    }
    if (action == START) {
        scope_fsm_start(n);
    } else {
        scope_fsm_end(n);
    }
    stc->current_scope = get_scope();
    stc->current_oc = get_overloading_class();
}

/*
 * initialize_fsm
 * Purpose:
 *      Prepare scope tracking finite state machine for use.
 * Parameters:
 *      None
 * Returns:
 *      None
 * Side Effects:
 *      Sets the values of static variables in this file
 */
void initialize_fsm() {
    set_state(TOP_LEVEL);
    set_scope(TOP_LEVEL_SCOPE);
    set_overloading_class(OTHER_NAMES);
    fsm_initialized = TRUE;
}

/*
 * scope_fsm_start
 *      update finite state machine representing program scope
 *      this method assumes that the caller just ENTERED the given node
 *      while traversing the parse tree.
 * parameters:
 *  n - pointer to the Node prompting the call to this function
 *
 * returns: none
 * side effects: updates internal representation of scope
 */
void scope_fsm_start(Node *n) {
    enum data_type nt = n->n_type;

    if (nt == FUNCTION_DEFINITION && get_state() == TOP_LEVEL) {
        set_state(FUNC_DEF);
    } else if (nt == FUNCTION_DECLARATOR && get_state() == FUNC_DEF) {
        set_state(FUNC_DEF_DECL);
    } else if (node_is_function_param(n) && get_state() == FUNC_DEF_DECL) {
        set_state(FUNC_DEF_PARAMS);
        new_scope();
    } else if (nt == COMPOUND_STATEMENT && get_state() == FUNC_DEF_DECL) {
        set_state(FUNC_BODY);
        new_scope();
    } else if (nt == COMPOUND_STATEMENT && get_state() == FUNC_BODY) {
        set_state(BLOCK);
        new_scope();
    } else if (nt == COMPOUND_STATEMENT && get_state() == BLOCK) {
        set_state(BLOCK);
        new_scope();
    }

    if (node_begins_statement_label(n)) {
        set_overloading_class(STATEMENT_LABELS);
    }
}

/*
 * scope_fsm_end
 *      update finite state machine representing program scope
 *      this method assumes that the caller just EXITED the given node
 *      while traversing the parse tree.
 * parameters:
 *  n - pointer to the Node prompting the call to this function
 *
 * returns: none
 * side effects: updates internal representation of scope
 */
void scope_fsm_end(Node *n) {
    enum data_type nt = n->n_type;

    if (node_is_function_param(n) && get_state() == FUNC_DEF_PARAMS) {
        set_state(FUNC_DEF_DECL);
        previous_scope();
    } else if (nt == COMPOUND_STATEMENT && get_state() == FUNC_BODY) {
        /* end of function definition */
        set_state(TOP_LEVEL);
        previous_scope();
    } else if (nt == COMPOUND_STATEMENT && get_state() == BLOCK) {
        previous_scope();
        if (scope == TOP_LEVEL_SCOPE) {
            set_state(TOP_LEVEL);
        } else if (scope == FUNCTION_SCOPE) {
            set_state(FUNC_BODY);
        } else {
            set_state(BLOCK);
        }
    }

    if (nt == NAMED_LABEL && get_overloading_class() == STATEMENT_LABELS) {
        set_overloading_class(OTHER_NAMES);
    }
}

enum Boolean fsm_is_ready() {
    return fsm_initialized;
}

static void new_scope() {
    scope++;
}

static void previous_scope() {
    scope--;
}

static void set_scope(int s) {
    scope = s;
}


static void set_state(int state) {
    current_state = state;
}

static void set_overloading_class(int oc) {
    overloading_class = oc;
}

int get_scope() {
    return scope;
}

int get_state() {
    return current_state;
}

int get_overloading_class() {
    return overloading_class;
}

/*
 * node_is_function_param
 *  determine if given node is a function parameter
 *
 * Parameters:
 *  n - pointer to the Node prompting the call to this function
 *
 * Return: 1 if it is a function parameter, 0 otherwise
 * Side effects: none
 *
 */
int node_is_function_param(Node *n) {
    return (n->n_type == PARAMETER_LIST || n->n_type == PARAMETER_DECL);
}

int node_begins_statement_label(Node *n) {
    return (n->n_type == LABELED_STATEMENT || n->n_type == GOTO_STATEMENT);
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
        CASE_FOR(FUNC_DEF);
        CASE_FOR(FUNC_DEF_DECL);
        CASE_FOR(FUNC_DEF_PARAMS);
        CASE_FOR(FUNC_BODY);
        CASE_FOR(BLOCK);
    #undef CASE_FOR
        default: return "";
  }
}

char *get_overloading_class_name(int oc) {
    switch (oc) {
    #define CASE_FOR(oc) case oc: return #oc
        CASE_FOR(OTHER_NAMES);
        CASE_FOR(STATEMENT_LABELS);
    #undef CASE_FOR
        default: return "";
  }
}

enum Boolean is_inner_block(int scope) {
    return scope > FUNCTION_SCOPE;
}
