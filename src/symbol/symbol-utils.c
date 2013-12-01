#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/symbol.h"
#include "../include/symbol-utils.h"
#include "../include/scope-fsm.h"
#include "../include/traverse.h"
#include "../include/literal.h"
#include "../include/utilities.h"

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
    /* create starting symbol tables */
    /* OTHER_NAMES file scope */
    stc->current_st[OTHER_NAMES] =
        create_symbol_table(TOP_LEVEL_SCOPE, OTHER_NAMES);
    /* STATEMENT_LABELS file scope. should stay empty */
    /* it exists to have an ST enclosing the function level ones */
    stc->current_st[STATEMENT_LABELS] =
        create_symbol_table(TOP_LEVEL_SCOPE, STATEMENT_LABELS);
    stc->function_prototypes = create_function_prototypes();
    stc->current_scope = TOP_LEVEL_SCOPE;
    stc->current_oc = OTHER_NAMES;
}

SymbolTable *new_current_st(int scope, int oc, SymbolTableContainer *stc) {
    SymbolTable *st = create_symbol_table(scope, oc);
    insert_symbol_table(st, stc);
    set_current_st(st, stc);
    return st;
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
    /* oc is OTHER_NAMES or STATEMENT_LABELS */
    int oc = st_overloading_class(new);
    SymbolTable *enc = stc->current_st[oc];
    if (enc == NULL) {
        stc->symbol_tables[oc] = new;
    }
    /* link new to its enclosing scope (even if it is NULL) */
    new->enclosing = enc;
}

void set_current_st(SymbolTable *st, SymbolTableContainer *stc) {
    int oc = st_overloading_class(st);
    stc->current_st[oc] = st;
}

SymbolTable *get_current_st(SymbolTableContainer *stc) {
    return stc->current_st[stc->current_oc];
}




/* symbol table */

/*
 * create_symbol_table
 */
SymbolTable *create_symbol_table(int scope, int overloading_class) {
    SymbolTable *st;
    util_emalloc( (void **) &st, sizeof(SymbolTable));
    initialize_st(st, scope, overloading_class);
    return st;
}

void initialize_st(SymbolTable *st, int scope, int overloading_class) {
    st->scope = scope;
    st->oc = overloading_class;
    st->symbols = NULL;
    st->enclosing = NULL;
}

SymbolTable *create_function_prototypes() {
    return create_symbol_table(TOP_LEVEL_SCOPE, OTHER_NAMES);
}

/* append the symbol s to the symbol table st */
void append_symbol(SymbolTable *st, Symbol *s) {
    Symbol *prev, *cur;
    prev = cur = st->symbols;
    /* check for duplicates */
    /* forward declarations, i.e. function prototypes, will only exist */
    /* in a separate symbol table so they will not be flagged here */
    while (cur != NULL) {
        if (strcmp(cur->name, s->name) == 0) {
            /* we do not return here to allow printing of invalid symbols */
            handle_symbol_error(STE_DUPLICATE_SYMBOL, s->name);
        }
        prev = cur;
        cur = cur->next;
    }
    /* no problems found. append the symbol */
    attach_symbol(st, s, prev);
}

void append_function_prototype(SymbolTable *prototypes, Symbol *s) {
    Symbol *prev, *cur;
    prev = cur = prototypes->symbols;
    /* duplicate prototypes are acceptable */
    /* TODO: check for duplicate prototypes that don't match exactly */
    while (cur != NULL) {
        prev = cur;
        cur = cur->next;
    }
    attach_symbol(prototypes, s, prev);
}

void attach_symbol(SymbolTable *st, Symbol *s, Symbol *prev) {
    if (prev != NULL) {
        prev->next = s;
    } else {
        st->symbols = s;
    }
    s->symbol_table = st;
}

Symbol *st_symbols(SymbolTable *st) {
    return st->symbols;
}

int st_scope(SymbolTable *st) {
    return st->scope;
}

int st_overloading_class(SymbolTable *st) {
    return st->oc;
}

char *st_scope_name(SymbolTable *st) {
    switch(st->scope) {
        case TOP_LEVEL_SCOPE:
            return "file";
        case FUNCTION_SCOPE:
            return "function";
        default:
            return "block";
    }
}

char *st_overloading_class_name(SymbolTable *st) {
    switch(st->oc) {
        case OTHER_NAMES:
            return "other names";
        case STATEMENT_LABELS:
            return "statement labels";
        default:
            return "";
    }
}

Symbol *find_prototype(SymbolTable *prototypes, char *name) {
    return find_symbol(prototypes, name);
}

Symbol *find_symbol(SymbolTable *st, char *name) {
    if (st == NULL) {
        return (Symbol *) NULL;
    }
    Symbol *cur = st->symbols;
    while (cur != NULL) {
        if (strcmp(cur->name, name) == 0) {
            return cur;
        }
        cur = cur->next;
    }
    return find_symbol(st->enclosing, name);
}



/* symbol */
Symbol *create_symbol() {
    Symbol *s;
    util_emalloc((void **) &s, sizeof(Symbol));
    s->name = "";
    s->type_tree = NULL;
    s->param_list = NULL;
    s->label_defined = FALSE;
    s->symbol_table = NULL;
    s->next = NULL;
    return s;
}

void push_symbol_type(Symbol *s, int t) {
    s->type_tree = push_type(s->type_tree, t);
}

int symbol_outer_type(Symbol *s) {
    return s->type_tree->type;
}

void set_symbol_name(Symbol *s, char *name) {
    s->name = name;
}

char *get_symbol_name(Symbol *s) {
    return s->name;
}

void set_label_defined(Symbol *s, Boolean b) {
    s->label_defined = b;
}

Boolean label_is_defined(Symbol *s) {
    return s->label_defined;
}

void set_symbol_func_params(Symbol *s, FunctionParameter *fp) {
    FunctionParameter *cur;
    if (symbol_outer_type(s) != FUNCTION) {
        handle_symbol_error(STE_NOT_FUNCTION, "adding param to non-function");
    }
    s->param_list = fp;
    cur = fp;
    while (cur != NULL) {
        s->type_tree->n.param_count++;
        cur = cur->next;
    }
}

SymbolTable *get_symbol_table(Symbol *s) {
    return s->symbol_table;
}

FunctionParameter *first_parameter(Symbol *s) {
    return s->param_list;
}

/* set the size of the first element of this symbol's type_tree */
void set_symbol_array_size(Symbol *s, int n) {
    if (symbol_outer_type(s) != ARRAY) {
        handle_symbol_error(STE_NOT_ARRAY, "setting array size on non-array");
    }
    set_array_size(s->type_tree, n);
}

Boolean all_array_bounds_specified(Symbol *s) {
    TypeNode *tn = s->type_tree;
    while (tn != NULL) {
        if (tn->type == ARRAY && tn->n.array_size == UNSPECIFIED_VALUE) {
            return FALSE;
        }
        tn = tn->next;
    }
    return TRUE;
}

Boolean symbols_same_type(Symbol *s1, Symbol *s2) {
    if (s1 == NULL || s2 == NULL) {
        if (s1 == NULL && s2 == NULL) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
    TypeNode *tn1 = s1->type_tree;
    TypeNode *tn2 = s2->type_tree;
    Boolean equal = equal_types(tn1, tn2);
    if (!equal) {
        return FALSE;
    }
    if (symbol_outer_type(s1) == FUNCTION) {
        /* and we know s2 is a FUNCTION from the check above */
        FunctionParameter *fp1 = first_parameter(s1);
        FunctionParameter *fp2 = first_parameter(s2);
        while (fp1 != NULL && fp2 != NULL) {
            equal = equal && parameters_same_type(fp1, fp2);
            if (!equal) {
                return FALSE;
            }
            fp1 = fp1->next;
            fp2 = fp2->next;
        }
        if (fp1 != NULL || fp2 != NULL) {
            return FALSE;
        }
    }
    return TRUE;
}

char *symbol_type_string(Symbol *s) {
    return type_tree_to_string(s->type_tree);
}




/* FunctionParameter */
FunctionParameter *create_function_parameter() {
    FunctionParameter *fp;
    util_emalloc((void **) &fp, sizeof(FunctionParameter));
    fp->name = "";
    return fp;
}

FunctionParameter *push_function_parameter(FunctionParameter *param_list) {
    FunctionParameter *fp = create_function_parameter();
    fp->next = param_list;
    return fp;
}

void set_parameter_name(FunctionParameter *fp, char *pname) {
    fp->name = pname;
}

char *get_parameter_name(FunctionParameter *fp) {
    return fp->name;
}

void push_parameter_type(FunctionParameter *fp, int t) {
    if (fp == NULL) {
        handle_symbol_error(STE_NULL_PARAM, "push_symbol_parameter_type");
        return;
    }
    fp->type_tree = push_type(fp->type_tree, t);
}

char *parameter_type_string(FunctionParameter *fp) {
    return type_tree_to_string(fp->type_tree);
}

Boolean parameters_same_type(FunctionParameter *fp1, FunctionParameter *fp2) {
    TypeNode *tn1 = fp1->type_tree;
    TypeNode *tn2 = fp2->type_tree;
    return equal_types(tn1, tn2);
}


/* TypeNode helpers */
TypeNode *create_type_node(int type) {
    TypeNode *tn;
    util_emalloc((void **) &tn, sizeof(TypeNode));
    tn->type = type;
    tn->next = NULL;
    return tn;
}

TypeNode *push_type(TypeNode *type_tree, int t) {
    TypeNode *tn = create_type_node(t);
    if (t == FUNCTION) {
        tn->n.param_count = 0;
    } else if (t == ARRAY) {
        tn->n.array_size = 0;
    }
    tn->next = type_tree;
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

Boolean equal_types(TypeNode *t1, TypeNode *t2) {
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
                    tempcnt = snprintf(bp, remaining_cur,
                                            " (unspecified size) of");
                } else {
                    num = get_array_size(tn);
                    tf = (num == 1 || num == -1);
                    paren_str = tf ? " (%d element) of" : " (%d elements) of";
                    tempcnt = snprintf(bp, remaining_cur, paren_str, num);
                }
                bp += tempcnt;
                remaining_cur -= tempcnt;
            } else if (tn->type == FUNCTION) {
                num = get_parameter_count(tn);
                tf = (num == 1 || num == -1);
                paren_str = tf ? " (%d parameter) returning" :
                                 " (%d parameters) returning";
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
            error(0, 0, "error: \"%s\": duplicate symbol", data);
            return;
        case STE_NON_POSITIVE_ARRAY_SIZE:
            error(0, 0, "error: %s: array size must be positive", data);
            return;
        case STE_VARIABLE_ARRAY_SIZE:
            error(0, 0, "error: %s: variable size not permitted", data);
            return;
        case STE_ARRAY_SIZE_TYPE:
            error(0, 0, "error: %s: array size must be an integer", data);
            return;
        case STE_ARRAY_SIZE_MISSING:
            error(0, 0, "error: %s: array size required", data);
            return;
        case STE_ARRAY_OF_FUNC:
            error(0, 0, "error: %s: arrays cannot contain functions", data);
            return;
        case STE_FUNC_RET_ARRAY:
            error(0, 0, "error: %s: functions cannot return arrays", data);
            return;
        case STE_FUNC_RET_FUNC:
            error(0, 0, "error: %s: functions cannot return functions", data);
            return;
        case STE_CAST_ARRAY_SIZE:
            error(0, 0, "error: %s: cast expressions not supported", data);
            return;
        case STE_NULL_PARAM:
            error(0, 0, "error: %s: trying to manipulate null parameter", data);
            return;
        case STE_FUNCTION_POINTER:
            error(0, 0, "error: %s: function pointers not supported", data);
            return;
        case STE_NOT_FUNCTION:
            error(0, 0, "error: %s", data);
        case STE_PROTO_MISMATCH:
            error(0, 0, "error: %s: redeclaration of function", data);
            return;
        case STE_FUNC_DECL_SCOPE:
            error(0, 0, "error: %s: function declared at non-file scope", data);
            return;
        case STE_ABS_DECL_PARAM:
            error(0, 0, "error: %s: function parameters must be named", data);
            return;
        case STE_ID_UNDECLARED:
            error(0, 0, "error: \"%s\" undeclared", data);
            return;
        case STE_LAB_UNDEFINED:
            error(0, 0, "error: '%s' used but not defined", data);
            return;
        default:
            return;
    }
}

