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

/* possible type categories of symbols */
enum type_category {
    SCALAR,
    ARRAY,
    FUNCTION
};

/* data structures for symbol table management */
struct TypeNode {
    int type;
    struct TypeNode *next;
};
typedef struct TypeNode TypeNode;

struct FunctionParameter {
    char *name;
    TypeNode *type_tree;
};
typedef struct FunctionParameter FunctionParameter;

union TypeData {
    TypeNode *scalar_type;
    TypeNode *return_type;
    TypeNode *element_type;
};

union SymbolMetadata {
    int param_count;
    int array_size;
};

/*
 * Symbol data structure
 * A symbol can represent a basic scalar value, a function, or an array.
 * This structure accommodates all three.
 */
struct Symbol {
    char *name;             /* the name of the symbol */
    int category;           /* type category: SCALAR, ARRAY, FUNCTION */
    union TypeData td;      /* type, function return type, array element type */
    union SymbolMetadata md;        /* function param count, array size */
    FunctionParameter *param_list;  /* function parameter list */
    struct Symbol *next;            /* adjacent item in symbol table */
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
Symbol *create_symbol();

/* error handling */
void handle_symbol_error(enum symbol_error e, char *data, int line);

#endif
