#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
            n->n_type == TYPE_SPECIFIER);
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
    stc->current_st = (SymbolTable *) NULL;
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

char *get_st_scope(SymbolTable *st) {
    switch(st->scope) {
        case TOP_LEVEL_SCOPE:
            return "file";
        case FUNCTION_BODY_SCOPE:
            return "function";
        default:
            return "block";
    }
}

char *get_st_overloading_class(SymbolTable *st) {
    switch(st->oc) {
        case OTHER_NAMES:
            return "other names";
        case STATEMENT_LABELS:
            return "statement labels";
        default:
            return "";
    }
}

Symbol *get_st_symbols(SymbolTable *st) {
    return st->symbols;
}

/* append the symbol s to the symbol table st */
void append_symbol(SymbolTable *st, Symbol *s) {
    Symbol *prev, *cur;
    prev = cur = st->symbols;
    while (cur != NULL) {
        /* check for duplicates */
        /* forward declarations, i.e. function prototypes, will only exist */
        /* in a separate symbol table so they will not be flagged here */
        if (strcmp(cur->name, s->name) == 0) {
            handle_symbol_error(STE_DUPLICATE_SYMBOL, "append_symbol");
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    if (prev != NULL) {
        prev->next = s;
    } else {
        st->symbols = s;
    }
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
    SymbolTable *enc = stc->current_st;
    if (enc == NULL) {
        /* new->oc is OTHER_NAMES or STATEMENT_LABELS */
        stc->symbol_tables[new->oc] = new;
    }
    /* link new to its enclosing scope (even if it is NULL) */
    new->enclosing = enc;
    stc->current_st = new;
}


/* Symbol functions */
Symbol *create_symbol() {
    Symbol *s;
    util_emalloc((void **) &s, sizeof(Symbol));
    s->name = "";
    s->type_tree = NULL;
    s->param_list = NULL;
    s->next = NULL;
    return s;
}

void push_symbol_type(Symbol *s, int t) {
    s->type_tree = push_type(s->type_tree, t);
}

void append_function_parameter_to_symbol(Symbol *s) {
    if (symbol_outer_type(s) != FUNCTION) {
        handle_symbol_error(STE_NOT_FUNCTION, "adding param to non-function");
    }
    FunctionParameter *fp = last_parameter(s);
    if (fp == NULL) {
        s->param_list = create_function_parameter();
        s->type_tree->n.param_count = 1;
        return;
    } else {
        fp->next = create_function_parameter();
        s->type_tree->n.param_count++;
    }
}

void push_symbol_parameter_type(Symbol *s, int t) {
    /* push onto the front of the last parameter of this symbol's param list */
    FunctionParameter *fp = last_parameter(s);
    if (fp == NULL) {
        handle_symbol_error(STE_NULL_PARAM, "push_symbol_parameter_type");
        return;
    }
    push_parameter_type(fp, t);
}

FunctionParameter *first_parameter(Symbol *s) {
    return s->param_list;
}

FunctionParameter *last_parameter(Symbol *s) {
     FunctionParameter *fp = s->param_list;
    if (fp == NULL) {
        return NULL;
    }
    while (fp->next != NULL) {
        fp = fp->next;
    }
    return fp;
}

/* set the size of the first element of this symbol's type_tree */
void set_symbol_array_size(Symbol *s, int n) {
    if (symbol_outer_type(s) != ARRAY) {
        handle_symbol_error(STE_NOT_ARRAY, "setting array size on non-array");
    }
    set_array_size(s->type_tree, n);
}

int symbol_outer_type(Symbol *s) {
    return s->type_tree->type;
}

enum Boolean symbols_same_type(Symbol *s1, Symbol *s2) {
    TypeNode *tn1 = s1->type_tree;
    TypeNode *tn2 = s2->type_tree;
    return equal_types(tn1, tn2);
}

void set_symbol_name(Symbol *s, char *name) {
    s->name = name;
}

char *get_symbol_name(Symbol *s) {
    return s->name;
}

char *symbol_type_string(Symbol *s) {
    return type_tree_to_string(s->type_tree);
}

enum Boolean all_array_bounds_specified(Symbol *s) {
    TypeNode *tn = s->type_tree;
    while (tn != NULL) {
        if (tn->type == ARRAY && tn->n.array_size == UNSPECIFIED_VALUE) {
            return FALSE;
        }
        tn = tn->next;
    }
    return TRUE;
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

char *get_parameter_name(FunctionParameter *fp) {
    return fp->name;
}

char *parameter_type_string(FunctionParameter *fp) {
    return type_tree_to_string(fp->type_tree);
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
    tn->next = NULL;
    return tn;
}

void set_array_size(TypeNode *tn, int size) {
    tn->n.array_size = size;
}

int get_array_size(TypeNode *tn) {
    if (tn->type != ARRAY) {
        handle_symbol_error(STE_NOT_ARRAY, "array size requested from non-array");
    }
    return tn->n.array_size;
}

int get_parameter_count(TypeNode *tn) {
    if (tn->type != FUNCTION) {
        handle_symbol_error(STE_NOT_FUNCTION,
                            "parameter count requested from non-function");
    }
    return tn->n.param_count;
}

/*
 *  type_tree_to_string
 *  Purpose:
 *      create a string showing the type chain starting from the given TypeNode
 *  Parameters:
 *      tn - the starting TypeNode, e.g. a Symbol's type_tree
 *  Returns:
 *      pointer to created string
 *  Side Effects:
 *      Allocates heap storage
 */
char *type_tree_to_string(TypeNode *tn) {
    /* buffer for printing type tree to and running pointer */
    char *buf, *bp, *paren_str;
    /* counters for ensuring we have enough space */
    int tempcnt, num, tf, remaining_cur, remaining_buf = MAX_TYPE_TREE_STRLEN;
    util_emalloc((void **) &buf, MAX_TYPE_TREE_STRLEN + 1);
    bp = buf;
    while (tn != NULL) {
        /* if there is room in the buffer for a type string, append this one */
        if (remaining_buf > MAX_TYPE_STRLEN) {
            remaining_cur = MAX_TYPE_STRLEN;
            if (bp != buf) {
                /* the string has a preceding item */
                tempcnt = snprintf(bp, remaining_cur, " -> ");
                bp += tempcnt;
                remaining_cur -= tempcnt;
            }
            tempcnt = snprintf(bp, remaining_cur, "%s",
                                get_type_tree_name(tn->type));
            bp += tempcnt;
            remaining_cur -= tempcnt;
            if (tn->type == ARRAY) {
                if (get_array_size(tn) == UNSPECIFIED_VALUE) {
                    tempcnt = snprintf(bp, remaining_cur, " (unspecified size)");
                } else {
                    num = get_array_size(tn);
                    tf = (num == 1 || num == -1);
                    paren_str = tf ? " (%d element)" : " (%d elements)";
                    tempcnt = snprintf(bp, remaining_cur, paren_str, num);
                }
                bp += tempcnt;
                remaining_cur -= tempcnt;
            } else if (tn->type == FUNCTION) {
                num = get_parameter_count(tn);
                tf = (num == 1 || num == -1);
                paren_str = tf ? " (%d parameter)" : " (%d parameters)";
                tempcnt = snprintf(bp, remaining_cur, paren_str, num);
                bp += tempcnt;
                remaining_cur -= tempcnt;
            }
            remaining_buf -= MAX_TYPE_STRLEN;
        }
        tn = tn->next;
    }
    return buf;
}

char *get_type_tree_name(int type) {
    char *type_name;
    /* char, int, long ,etc. */
    type_name = util_get_type_spec(type);
    if (strcmp(type_name, "") == 0) {
        /* array, function */
        type_name = get_type_category_name(type);
    }
    return type_name;
}

char *get_type_category_name(int type) {
    switch (type) {
        case FUNCTION:
            return "function";
        case ARRAY:
            return "array";
        default:
            return "";
    }
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
 * Returns:
 *      None
 * Side effects:
 *      May terminate program depending on error type
 */
void handle_symbol_error(enum symbol_error e, char *data) {
    switch (e) {
        case STE_SUCCESS:
            return;
        case STE_NOT_ARRAY:
            error(0, 0, "%s", data);
            return;
        case STE_DUPLICATE_SYMBOL:
            error(0, 0, "%s: duplicate symbol", data);
            return;
        case STE_NON_POSITIVE_ARRAY_SIZE:
            error(0, 0, "%s: array size must be positive", data);
            return;
        case STE_VARIABLE_ARRAY_SIZE:
            error(0, 0, "%s: variable size not permitted", data);
            return;
        case STE_ARRAY_SIZE_TYPE:
            error(0, 0, "%s: array size must be an integer", data);
            return;
        case STE_ARRAY_SIZE_MISSING:
            error(0, 0, "%s: array size required", data);
            return;
        case STE_ARRAY_OF_FUNC:
            error(0, 0, "%s: arrays cannot contain functions", data);
            return;
        case STE_FUNC_RET_ARRAY:
            error(0, 0, "%s: functions cannot return arrays", data);
            return;
        case STE_FUNC_RET_FUNC:
            error(0, 0, "%s: functions cannot return functions", data);
            return;
        case STE_CAST_ARRAY_SIZE:
            error(0, 0, "%s: cast expression array bounds not supported", data);
            return;
        case STE_NULL_PARAM:
            error(0, 0, "%s: trying to manipulate null parameter", data);
            return;
        case STE_FUNCTION_POINTER:
            error(0, 0, "%s: function pointers not supported", data);
            return;
        case STE_NOT_FUNCTION:
            error(0, 0, "%s", data);
            return;
        default:
            return;
    }
}

