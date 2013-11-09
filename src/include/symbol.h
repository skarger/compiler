
#include "parse-tree.h"
#include "literal.h"

#ifndef SYMBOL_H
#define SYBMOL_H

struct symbol {
    char *name;
    enum data_type type;
    struct symbol_table *enclosing;
    struct symbol *next;
};

struct SymbolTable {
    int scope;
    struct symbol *symbols;
};

typedef struct SymbolTable SymbolTable;

enum scope_state {
    TOP_LEVEL,
    FUNCTION_DEF,
    FUNCTION_DEF_PARAMETERS,
    FUNCTION_BODY,
    FUNCTION_PROTOTYPE,
    FUNCTION_PROTO_PARAMETERS,
    BLOCK
};

/*
 * Errors that are caught in the symbol table step.
 */
enum symbol_error {
    STE_SUCCESS = 0,
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


/* finite state machine functions */
int get_state();
int get_scope();
int get_overloading_class();
void initialize_fsm();
void transition_scope(Node *n, int action);

/* symbol table functions */
SymbolTable *create_symbol_table();
void set_st_scope(SymbolTable *st, int scope);
void set_st_symbols(SymbolTable *st, struct symbol *s);
enum Boolean should_create_new_st();

/* error handling */
void handle_symbol_error(enum symbol_error e, char *data, int line);

#endif
