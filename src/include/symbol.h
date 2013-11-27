#include "parse-tree.h"
#include "literal.h"

#ifndef SYMBOL_H
#define SYMBOL_H

/* give names to fundamental scope levels. deeper levels need no name */
#define TOP_LEVEL_SCOPE 0
#define FUNCTION_BODY_SCOPE 1
#define STATEMENT_LABEL_SCOPE 1

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

/* array bound related */
/* choosing these constants since they are invalid array bounds anyway */
#define UNSPECIFIED_VALUE   -2147483647
#define VARIABLE_VALUE      -2147483646
#define NON_INTEGRAL_VALUE  -2147483645
#define CAST_VALUE          -2147483644

/* function related */
#define PARAM_NAME 0
#define PARAM_TYPE 1

/* limit on the length of a type tree chain in string format */
#define MAX_TYPE_TREE_STRLEN 511
/*
limit on individual type strings, e.g.
"unsigned long -> "
"array (1234567890 elements) -> "
"array (unspecified size) ->"
"function (12345 parameters) -> "
*/
#define MAX_TYPE_STRLEN 32


/* data structures for symbol table management */
union TypeNumericValue {
    int param_count;
    int array_size;
};

/*
 * TypeNode
 * Constitutes a "type tree", either by itself or chained with other TypeNodes
 *
 * It represents the type of either:
 *  a scalar value
 *  a function return type
 *  a function parameter type
 *  an array element type
 *
 * Each TypeNode has a type value which can be either:
 *  a category type: SCALAR, ARRAY, FUNCTION
 *  a canonical integral type: SIGNED_CHAR, ..., UNSIGNED_LONG
 *  a pointer type
 *
 * It may have a numeric value, which represents either:
 *  the number of elements in an ARRAY
 *  the number of parameters to a FUNCTION
 */
struct TypeNode {
    int type;
    union TypeNumericValue n;
    struct TypeNode *next;
};
typedef struct TypeNode TypeNode;

struct FunctionParameter {
    char *name;
    TypeNode *type_tree;
    struct FunctionParameter *next;
};
typedef struct FunctionParameter FunctionParameter;

/*
 * Symbol data structure
 * A symbol can represent a basic scalar value, a function, or an array.
 * This structure accommodates all three.
 */
struct Symbol {
    char *name;          /* the name of the symbol */
    TypeNode *type_tree;
    FunctionParameter *param_list;      /* function parameter list */
    struct Symbol *next;                /* adjacent item in symbol table */
    struct SymbolTable *symbol_table;   /* the symbol's symbol table */
};
typedef struct Symbol Symbol;

/*
 * SymbolTable
 * Maintains a list of symbols as well as the scope and overloading class
 * of those symbols.
 * Also has a pointer to the SymbolTable that encloses it.
 * A source file will usually prompt the creation of several SymbolTables
 */
struct SymbolTable {
    Symbol *symbols;                /* list of symbols in symbol table     */
    int scope;                      /* scope level (file, function, etc.)  */
    int oc;                         /* overloading class                   */
    struct SymbolTable *enclosing;  /* symbol table at enclosing scope     */
};
typedef struct SymbolTable SymbolTable;

/*
 * SymbolTableContainer
 * The single container for all the SymbolTables created while
 * compiling a source file

                       s2                s2                    s2
                       |                 |                     |
                       s1                s1                    s1
                       |                 |                     |
     | OTHER_NAMES:  file, scope 0  -- function 1, scope 1 -- block, scope 2
     |                            \
     |                             \ - function 2, scope 1
symbol_tables                                 |
     |                                        s1
     |
     | STATEMENT_LABELS: file, scope 0 (no statement label symbols)
                                     \
                                       - function 1, scope 1  <- current_st
                                            |
                                            s1
                                            |
                                            s2

function_prototypes: f1 -- f2 -- f3

*/
struct SymbolTableContainer {
    /* contains the root symbol tables for each overloading class */
    SymbolTable *symbol_tables[NUM_OC_CLASSES];
    /* points to the last symbol table appended */
    SymbolTable *current_st;
    /* separate symbol table for tracking function declarations */
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
    STE_NOT_ARRAY = -1,
    STE_DUPLICATE_SYMBOL = -2,
    STE_NON_POSITIVE_ARRAY_SIZE = -3,
    STE_VARIABLE_ARRAY_SIZE = -4,
    STE_ARRAY_SIZE_TYPE = -5,
    STE_ARRAY_SIZE_MISSING = -6,
    STE_ARRAY_OF_FUNC = 7,
    STE_FUNC_RET_ARRAY = -8,
    STE_FUNC_RET_FUNC = -9,
    STE_CAST_ARRAY_SIZE = -10,
    STE_NULL_PARAM = -11,
    STE_FUNCTION_POINTER = -12,
    STE_NOT_FUNCTION = -13,
    STE_PROTO_MISMATCH = -14
};


/* finite state machine functions */
int get_state();
int get_scope();
int get_overloading_class();
void transition_scope(Node *n, int action);

/* symbol table management functions */
SymbolTableContainer *create_st_container();
SymbolTable *create_symbol_table();
SymbolTable *create_function_prototypes();
void set_st_symbols(SymbolTable *st, Symbol *s);
enum Boolean should_create_new_st();
void insert_symbol_table(SymbolTable *new, SymbolTableContainer *stc);
char *get_st_overloading_class(SymbolTable *st);
char *get_st_scope(SymbolTable *st);
Symbol *get_st_symbols(SymbolTable *st);

Symbol *create_symbol();
Symbol *create_scalar_symbol();
Symbol *create_function_symbol();
Symbol *create_array_symbol();
int get_function_parameter_count(Symbol *s);
enum Boolean symbols_same_type(Symbol *s1, Symbol *s2);
void append_symbol(SymbolTable *st, Symbol *s);
void set_symbol_name(Symbol *s, char *name);
char *get_symbol_name(Symbol *s);
SymbolTable *get_symbol_table(Symbol *s);
char *symbol_type_string(Symbol *s);
enum Boolean all_array_bounds_specified(Symbol *s);
int symbol_outer_type(Symbol *s);
void append_function_parameter_to_symbol(Symbol *s);
Symbol *find_prototype(SymbolTable *prototypes, char *name);
Symbol *find_symbol(SymbolTable *st, char *name);

/* helpers */
TypeNode *create_type_node(int type);
TypeNode *push_type(TypeNode *type_tree, int t);
enum Boolean equal_types(TypeNode *t1, TypeNode *t2);
char *get_type_tree_name(int type);
char *get_type_category_name(int type);
char *type_tree_to_string(TypeNode *tn);
void set_array_size(TypeNode *tn, int size);
int get_array_size(TypeNode *tn);
int get_parameter_count(TypeNode *tn);


FunctionParameter *create_function_parameter();
FunctionParameter *first_parameter(Symbol *);
FunctionParameter *last_parameter(Symbol *s);
void set_function_parameter_name(FunctionParameter *fp, char *pname);
void push_parameter_type(FunctionParameter *fp, int t);
enum Boolean parameters_same_type(FunctionParameter *fp1, FunctionParameter *fp2);



/* error handling */
void handle_symbol_error(enum symbol_error e, char *data);

#endif
