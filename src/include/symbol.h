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
    char *name;          /* the name of the symbol */
    int category;        /* type category: SCALAR, ARRAY, FUNCTION */
    TypeNode *type_tree; /* basic type, func return type, array element type */
    union SymbolMetadata meta;      /* function param count, array size */
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
/*
                           s2                s2                    s2
                           |                 |                     |
                           s1                s1                    s1
                           |                 |                     |
         _ OTHER_NAMES:  file, scope 0  -- function 1, scope 1 -- block, scope 2
       /                              \
      /                                \ - function 2, scope 1
     /                                          |
STC |                                           s1
     \
      \ _ STATEMENT_LABELS: file, scope 0 (no statement label symbols)
                                         \
                                           - function 1, scope 1  <- current
                                                |
                                                s1
                                                |
                                                s2

function_prototypes: f1 -- f2 -- f3

symbol_tables: contains the root symbol tables for each overloading class
current: points to the last symbol table appended
function_prototypes: separate symbol table for tracking function declarations
*/
    SymbolTable *symbol_tables[NUM_OC_CLASSES];
    SymbolTable *current;
    SymbolTable *function_prototypes;
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
Symbol *create_scalar_symbol();
Symbol *create_function_symbol();
Symbol *create_array_symbol();
int get_function_parameter_count(Symbol *s);
enum Boolean symbols_same_type(Symbol *s1, Symbol *s2);
int get_array_size(Symbol *s);


/* helpers */
TypeNode *create_type_node(int type);
TypeNode *push_type(TypeNode *type_tree, int t);
enum Boolean equal_types(TypeNode *t1, TypeNode *t2);

FunctionParameter *create_function_parameter();
void set_function_parameter_name(FunctionParameter *fp, char *pname);
void push_parameter_type(FunctionParameter *fp, int t);
enum Boolean parameters_same_type(FunctionParameter *fp1, FunctionParameter *fp2);

int get_symbol_category(Symbol *s);

/* error handling */
void handle_symbol_error(enum symbol_error e, char *data, int line);

#endif
