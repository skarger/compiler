#include "parse-tree.h"
#include "literal.h"
#include "symbol.h"

#ifndef SYMBOL_UTILS_H
#define SYMBOL_UTILS_H

/* for testing */
#define PASS 1
#define FAIL 2


/* limit on the length of a type tree chain in string format */
#define MAX_TYPE_TREE_STRLEN 1023
/*
 * limit on individual type strings, e.g.
 * "unsigned long -> "
 * "array (1234567890 elements) of -> "
 * "array (unspecified size) ->"
 * "function (12345 parameters) returning -> "
 * */
#define MAX_TYPE_STRLEN 48

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
    STE_PROTO_MISMATCH = -14,
    STE_FUNC_DECL_SCOPE = -15,
    STE_ABS_DECL_FUNC = -16,
    STE_ID_UNDECLARED = -17
};


/* symbol table management functions */
SymbolTableContainer *create_st_container();
SymbolTable *create_symbol_table(int scope, int overloading_class);
SymbolTable *new_current_st(int scope, int oc, SymbolTableContainer *stc);
SymbolTable *create_function_prototypes();
void set_st_symbols(SymbolTable *st, Symbol *s);
void insert_symbol_table(SymbolTable *new, SymbolTableContainer *stc);
void set_current_st(SymbolTable *st, SymbolTableContainer *stc);
SymbolTable *get_current_st(SymbolTableContainer *stc);
int st_scope(SymbolTable *st);
char *st_scope_name(SymbolTable *st);
int st_overloading_class(SymbolTable *st);
char *st_overloading_class_name(SymbolTable *st);

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
void set_symbol_func_params(Symbol *s, FunctionParameter *fp);
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
void set_parameter_name(FunctionParameter *fp, char *pname);
char *get_parameter_name(FunctionParameter *fp);
FunctionParameter *push_function_parameter(FunctionParameter *param_list);
void push_parameter_type(FunctionParameter *fp, int t);
enum Boolean parameters_same_type(FunctionParameter *fp1, FunctionParameter *fp2);



/* error handling */
void handle_symbol_error(enum symbol_error e, char *data);

#endif
