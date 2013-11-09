#include "parse-tree.h"
#include "literal.h"

#ifndef SYMBOL_H
#define SYMBOL_H

#define TOP_LEVEL_SCOPE 0
#define FUNCTION_BODY_SCOPE 1

/* indicate whether we are starting or ending the traversal of a node */
#define START 1
#define END 2

/* for testing */
#define PASS 1
#define FAIL 2

/* overloading classes */
#define NUM_OC_CLASSES 2
#define OTHER_NAMES 0
#define STATEMENT_LABELS 1

struct symbol {
    char *name;
    enum data_type type;
    struct symbol_table *enclosing;
    struct symbol *next;
};

struct SymbolTable {
    struct symbol *symbols; /* list of symbols in symbol table    */
};
typedef struct SymbolTable SymbolTable;

struct ScopeSet {
    int scope;              /* scope level (file, function, etc.)    */
    int oc;                 /* overloading class                     */
    SymbolTable *st;       /* list of symbol tables at this scope/oc */
};
typedef struct ScopeSet ScopeSet;

struct SymbolTableContainer {
    ScopeSet *scope_set[NUM_OC_CLASSES];
};
typedef struct SymbolTableContainer SymbolTableContainer;


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


/* finite state machine functions */
int get_state();
int get_scope();
int get_overloading_class();
void initialize_fsm();
void transition_scope(Node *n, int action);

/* symbol table container functions */
SymbolTableContainer *create_st_container();
void initialize_st_container(SymbolTableContainer *stc);

/* scope set functions */
ScopeSet *create_scope_set();
void set_ss_scope(ScopeSet *ss, int scope);
void set_ss_overloading_class(ScopeSet *ss, int oc);

/* symbol table functions */
SymbolTable *create_symbol_table();
void set_st_symbols(SymbolTable *st, struct symbol *s);
enum Boolean should_create_new_st();

/* error handling */
void handle_symbol_error(enum symbol_error e, char *data, int line);

#endif
