#include <stdio.h>
#include "../include/symbol.h"
#include "../include/parse-tree.h"
#include "../include/literal.h"
#include "../include/utilities.h"
#include "../../y.tab.h"


/*
 * define a finite state machine to help with scope determination
 * while traversing parse tree.
 */

/*
* transitions
* Functions may be declared only at file scope.
TOP_LEVEL -> FUNCTION_DEF
    node: FUNCTION_DEFINITION
FUNCTION_DEF -> FUNCTION_DEF_PARAMETERS
    node: (PARAMETER_LIST | PARAMETER_DECL | TYPE_SPECIFIER == VOID)
FUNCTION_DEF_PARAMETERS -> FUNCTION_BODY
    node: start of COMPOUND_STATEMENT
FUNCTION_BODY -> TOP_LEVEL
    node: end of COMPOUND_STATEMENT

FUNCTION_BODY -> BLOCK
    node: start of COMPOUND_STATEMENT
BLOCK -> FUNCTION_BODY
    node: end of COMPOUND_STATEMENT

BLOCK -> BLOCK
    node: start of COMPOUND_STATEMENT
BLOCK -> BLOCK
    node: end of COMPOUND_STATEMENT

OTHER_NAMES -> STATEMENT_LABELS
    node: ( LABELED_STATEMENT | GOTO_STATEMENT )
STATEMENT_LABELS -> OTHER_NAMES
    node: IDENTIFIER

*/

static int current_state = TOP_LEVEL;
static int scope = TOP_LEVEL;
static int overloading_class = OTHER_NAMES;

/* upon entering a new scope level we should create a new symbol table */
/* this may occur more than once at that scope level (siblings) */
static enum Boolean create_new_st = TRUE;

/* helper functions */

/* scope finite state machine */
void scope_fsm_start(Node *n);
void scope_fsm_end(Node *n);
int node_is_function_param(Node *);
int node_begins_statement_label(Node *n);
char *get_scope_state_name(enum scope_state);
char *get_overloading_class_name(int oc);
/* symbol table */
void initialize_st_container(SymbolTableContainer *stc);
void initialize_st(SymbolTable *st);

static void set_state(int state) {
    current_state = state;
}

static void new_scope() {
    scope++;
    create_new_st = TRUE;
}

static void set_scope(int s) {
    scope = s;
}

static void set_overloading_class(int oc) {
    overloading_class = oc;
}

static void previous_scope() {
    scope--;
}

static void complete_st_creation() {
    create_new_st = FALSE;
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

enum Boolean should_create_new_st() {
    return create_new_st;
}

void initialize_fsm() {
    set_state(TOP_LEVEL);
    set_scope(TOP_LEVEL_SCOPE);
    set_overloading_class(OTHER_NAMES);
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
void transition_scope(Node *n, int action) {
    if (action == START) {
        scope_fsm_start(n);
    } else {
        scope_fsm_end(n);
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
    enum node_type nt = n->n_type;

    if (nt == FUNCTION_DEFINITION && get_state() == TOP_LEVEL) {
        set_state(FUNCTION_DEF);
    } else if (node_is_function_param(n) && get_state() == FUNCTION_DEF) {
        new_scope();
        set_state(FUNCTION_DEF_PARAMETERS);
    } else if (nt == COMPOUND_STATEMENT && get_state() == FUNCTION_DEF_PARAMETERS) {
        set_state(FUNCTION_BODY);
        /* already entered the new scope for FUNCTION_DEF_PARAMETERS */
    } else if (nt == COMPOUND_STATEMENT && get_state() == FUNCTION_BODY) {
        new_scope();
        set_state(BLOCK);
    } else if (nt == COMPOUND_STATEMENT && get_state() == BLOCK) {
        new_scope();
        set_state(BLOCK);
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
    enum node_type nt = n->n_type;

    if (nt == COMPOUND_STATEMENT && get_state() == FUNCTION_BODY) {
        /* end of function definition */
        previous_scope();
        set_state(TOP_LEVEL);
    } else if (nt == COMPOUND_STATEMENT && get_state() == BLOCK) {
        previous_scope();
        if (scope <= FUNCTION_BODY_SCOPE) {
            set_state(FUNCTION_BODY);
        } else {
            set_state(BLOCK);
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
    return (n->n_type == PARAMETER_LIST || n->n_type == PARAMETER_DECL ||
        (n->n_type == TYPE_SPECIFIER && n->data.attributes[TYPE_SPEC] == VOID));
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
        CASE_FOR(FUNCTION_DEF);
        CASE_FOR(FUNCTION_DEF_PARAMETERS);
        CASE_FOR(FUNCTION_BODY);
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

/* symbol table container */
SymbolTableContainer *create_st_container() {
    SymbolTableContainer *stc;
    util_emalloc( (void **) &stc, sizeof(SymbolTableContainer));
    initialize_st_container(stc);
    return stc;
}

void initialize_st_container(SymbolTableContainer *stc) {
    stc->symbol_tables[OTHER_NAMES] = (SymbolTable *) NULL;
    stc->symbol_tables[STATEMENT_LABELS] = (SymbolTable *) NULL;
    stc->current = (SymbolTable *) NULL;
    stc->function_prototypes = NULL;
}

/* symbol table */
/*
 * create_symbol_table
 */
SymbolTable *create_symbol_table() {
    SymbolTable *st;
    util_emalloc( (void **) &st, sizeof(SymbolTable));
    initialize_st(st);
    complete_st_creation();
    return st;
}

void initialize_st(SymbolTable *st) {
    st->scope = get_scope();
    st->oc = get_overloading_class();
    st->symbols = NULL;
}

void set_st_symbols(SymbolTable *st, Symbol *s) {
    st->symbols = s;
}

/*
 * insert_symbol_table
 * Purpose:
 *      Insert a new symbol table into the ST container, linking it to its
 *      enclosing scope.
 *      The new ST is either first in its overloading class (largest scope)
 *      or it has a non-null enclosing scope.
 *
 * Parameters:
 *      new - the new symbol table to insert
 *      stc - the symbol table container.
 *
 * Returns:
 *      None
 * Side Effects:
 *      Updates the current symbol table pointer of the ST container.
 */
void insert_symbol_table(SymbolTable *new, SymbolTableContainer *stc) {
    SymbolTable *enc = stc->current;
    if (enc == NULL) {
        /* new->oc is OTHER_NAMES or STATEMENT_LABELS */
        stc->symbol_tables[new->oc] = new;
    }
    /* link new to its enclosing scope (even if it is NULL) */
    new->enclosing = enc;
    stc->current = new;
}


/* Symbol functions */
Symbol *create_symbol() {
    Symbol *s;
    util_emalloc((void **) &s, sizeof(Symbol));
    s->name = "";
    s->type_tree = NULL;
    s->next = NULL;
    return s;
}

Symbol *create_scalar_symbol() {
    Symbol *s = create_symbol();
    s->category = SCALAR;
    return s;
}

Symbol *create_function_symbol() {
    Symbol *s = create_symbol();
    s->category = FUNCTION;
    s->meta.param_count = 0;
    s->param_list = NULL;
    return s;
}

Symbol *create_array_symbol() {
    Symbol *s = create_symbol();
    s->category = ARRAY;
    s->meta.array_size = 0;
}

int get_symbol_category(Symbol *s) {
    return s->category;
}

int get_function_parameter_count(Symbol *s) {
    if (s->category != FUNCTION) {
        return -1;
    }
    return s->meta.param_count;
}

int get_array_size(Symbol *s) {
    if (s->category != ARRAY) {
        return -1;
    }
    return s->meta.array_size;
}

void push_symbol_type(Symbol *s, int t) {
    s->type_tree = push_type(s->type_tree, t);
}

enum Boolean symbols_same_type(Symbol *s1, Symbol *s2) {
    TypeNode *tn1 = s1->type_tree;
    TypeNode *tn2 = s2->type_tree;
    return equal_types(tn1, tn2);
}


/* FunctionParameter helpers */
FunctionParameter *create_function_parameter() {
    FunctionParameter *fp;
    util_emalloc((void **) &fp, sizeof(FunctionParameter));
    fp->name = "";
    return fp;
}

void set_function_parameter_name(FunctionParameter *fp, char *pname) {
    fp->name = pname;
}

void push_parameter_type(FunctionParameter *fp, int t) {
    fp->type_tree = push_type(fp->type_tree, t);
}

enum Boolean parameters_same_type(FunctionParameter *fp1, FunctionParameter *fp2) {
    TypeNode *tn1 = fp1->type_tree;
    TypeNode *tn2 = fp2->type_tree;
    return equal_types(tn1, tn2);
}

/* TypeNode helpers */
TypeNode *push_type(TypeNode *type_tree, int t) {
    TypeNode *tn = create_type_node(t);
    tn->next = type_tree;
    return tn;
}

TypeNode *create_type_node(int type) {
    TypeNode *tn;
    util_emalloc((void **) &tn, sizeof(TypeNode));
    tn->type = type;
    return tn;
}

enum Boolean equal_types(TypeNode *t1, TypeNode *t2) {
    while(t1 != NULL && t2 != NULL) {
        if (t1->type != t2->type) {
            return FALSE;
        }
        t1 = t1->next;
        t2 = t2->next;
    }
    /* exited while loop so at least one must be NULL */
    /* if the other is not then it's different */
    if (t1 != NULL || t2 != NULL) {
        return FALSE;
    }
    return TRUE;
}

/*
 * handle_symbol_error
 * Purpose:
 *      Handle an error caught in the calling method.
 * Parameters:
 *      e - the error value.
 *      data - string that will be inserted into the message printed to stderr
 *      line - line number causing error, if applicable (e.g. from input source)
 * Returns:
 *      None
 * Side effects:
 *      May terminate program depending on error type
 */
void handle_symbol_error(enum symbol_error e, char *data, int line) {
    switch (e) {
        case STE_SUCCESS:
            return;
        default:
            return;
    }
}

