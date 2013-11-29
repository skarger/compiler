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

/* upon entering a new scope level we should create a new symbol table */
/* this may occur more than once at that scope level (siblings) */
/* also we must track the need to create a new symbol table separately */
/* for statement labels and other names */
static enum Boolean create_new_st[NUM_OC_CLASSES];
/* upon exiting a scope level we should start referencing  */
/* the symbol table at the enclosing scope but again that applies */
/* differently to the separate overloading classes */
static enum Boolean return_to_enclosing_st[NUM_OC_CLASSES];


/* scope finite state machine */
void initialize_fsm();
enum Boolean fsm_is_ready();
void scope_fsm_start(Node *n);
void scope_fsm_end(Node *n);
int node_is_function_param(Node *);
int node_begins_statement_label(Node *n);
char *get_scope_state_name(enum scope_state);
char *get_overloading_class_name(int oc);

static void set_state(int state) {
    current_state = state;
}

static void new_scope(enum scope_state state) {
    /* only create a new statement label scope when entering function body */
    if (state == FUNC_BODY) {
        create_new_st[STATEMENT_LABELS] = TRUE;
    }
    /* always create a new other names symbol table except for function */
    /* body because the needed ST was already created for its params */

/* fix to keep one ST for a param list */

    if (state != FUNC_BODY) {
        create_new_st[OTHER_NAMES] = TRUE;
    }
    scope++;
}

static void set_scope(int s) {
    scope = s;
}

static void set_overloading_class(int oc) {
    overloading_class = oc;
}

static void previous_scope(enum scope_state state) {
    /* don't back up while still in param list */

        return_to_enclosing_st[OTHER_NAMES] = TRUE;

    if (state == TOP_LEVEL) {
        return_to_enclosing_st[STATEMENT_LABELS] = TRUE;
    }
    scope--;
}

int get_state() {
    return current_state;
}

int get_scope() {
    return scope;
}

int get_overloading_class() {
    return overloading_class;
}

static enum Boolean should_create_new_st() {
    return create_new_st[get_overloading_class()];
}

static void complete_st_creation() {
    create_new_st[get_overloading_class()] = FALSE;
}

static enum Boolean should_return_to_enclosing_st() {
    return return_to_enclosing_st[get_overloading_class()];
}

static void complete_return_to_enclosing_st() {
    return_to_enclosing_st[get_overloading_class()] = FALSE;
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
    create_new_st[OTHER_NAMES] = FALSE;
    create_new_st[STATEMENT_LABELS] = FALSE;
    fsm_initialized = TRUE;
}

enum Boolean fsm_is_ready() {
    return fsm_initialized;
}

/*
 * transition_scope
 *      update program scope in response to the given node
 * parameters:
 *  n - pointer to the Node prompting the call to this function
 *  action - START or END, depending on what point of the parse tree traversal
 *                         the caller is one with the given node
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
    stc->current_oc = get_overloading_class();
    /* if we must create a new symbol table it should become the current one */
    /* or, if we moved to an enclosing scope then we must update the         */
    /* current symbol table pointer to the one enclosing the old one         */
    if (should_create_new_st()) {
        SymbolTable *st = create_symbol_table(get_scope(), stc->current_oc);
        complete_st_creation();
        insert_symbol_table(st, stc);
        set_current_st(st, stc);
    } else if (should_return_to_enclosing_st()) {
            set_current_st(stc->current_st[stc->current_oc]->enclosing, stc);
            complete_return_to_enclosing_st();
    }
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
        new_scope(FUNC_DEF_PARAMS);
    } else if (nt == COMPOUND_STATEMENT && get_state() == FUNC_DEF_DECL) {
        set_state(FUNC_BODY);
        new_scope(FUNC_BODY);
    } else if (nt == COMPOUND_STATEMENT && get_state() == FUNC_BODY) {
        set_state(BLOCK);
        new_scope(BLOCK);
    } else if (nt == COMPOUND_STATEMENT && get_state() == BLOCK) {
        set_state(BLOCK);
        new_scope(BLOCK);
    } else if (node_begins_statement_label(n)) {
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
        previous_scope(FUNC_DEF_DECL);
    } else if (nt == COMPOUND_STATEMENT && get_state() == FUNC_BODY) {
        /* end of function definition */
        set_state(TOP_LEVEL);
        previous_scope(TOP_LEVEL);
    } else if (nt == COMPOUND_STATEMENT && get_state() == BLOCK) {
        if (scope <= FUNCTION_BODY_SCOPE) {
            set_state(FUNC_BODY);
            previous_scope(FUNC_BODY);
        } else {
            set_state(BLOCK);
            previous_scope(BLOCK);
        }
    } else if (nt == IDENTIFIER && get_overloading_class() == STATEMENT_LABELS) {
        set_overloading_class(OTHER_NAMES);
    }
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
