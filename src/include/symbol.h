#include "parse-tree.h"
#include "literal.h"

#ifndef SYMBOL_H
#define SYMBOL_H

/* give names to fundamental scope levels. deeper levels need no name */
#define TOP_LEVEL_SCOPE 0
#define FUNCTION_BODY_SCOPE 1

/* indicator for whether we are starting or ending the traversal of a node */
#define START 1
#define END 2

/* for testing */
#define PASS 1
#define FAIL 2

/* overloading classes */
#define NUM_OC_CLASSES 2
#define OTHER_NAMES 0
#define STATEMENT_LABELS 1

/* data structures for symbol table management */
struct Symbol {
    char *name;
    /* type tree */
    struct Symbol *next;
};
typedef struct Symbol Symbol;

struct SymbolTable {
    Symbol *symbols;                /* list of symbols in symbol table     */
    int scope;                      /* scope level (file, function, etc.)  */
    int oc;                         /* overloading class                   */
    struct SymbolTable *enclosing;  /* symbol table at enclosing scope     */
};
typedef struct SymbolTable SymbolTable;

struct SymbolTableContainer {
    SymbolTable *symbol_tables[NUM_OC_CLASSES];
    SymbolTable *current;
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

/* symbol table management functions */
SymbolTableContainer *create_st_container();
SymbolTable *create_symbol_table();
void set_st_symbols(SymbolTable *st, Symbol *s);
enum Boolean should_create_new_st();
void insert_symbol_table(SymbolTable *new, SymbolTableContainer *stc);


/* error handling */
void handle_symbol_error(enum symbol_error e, char *data, int line);

#endif
