
#ifndef SYMBOL_H
#define SYMBOL_H

#include "utilities.h"

/* overloading classes */
#define NUM_OC_CLASSES 2
#define OTHER_NAMES 0
#define STATEMENT_LABELS 1


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
    Boolean label_defined;              /* statement label flag */
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
    SymbolTable *current_st[NUM_OC_CLASSES];
    /* separate symbol table for tracking function declarations */
    SymbolTable *function_prototypes;
    /* track the current scope */
    int current_scope;
    /* track the current overloading class */
    int current_oc;
};
typedef struct SymbolTableContainer SymbolTableContainer;


#endif
