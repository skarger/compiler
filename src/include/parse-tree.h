/* contains data definitions for parse tree nodes. */
#ifndef PARSE_TREE_H
#define PARSE_TREE_H

/* output file for printing results */
extern FILE *output;

/* Nodes have a type corresponding to the grammar. Sometimes the type is
 * equivalent to a lexical token, but often the type has a higher-level
 * semantic value that the parser ascertains.
 */
enum node_type {
    FUNCTION_DEFINITION,
    FUNCTION_DEF_SPEC,
    DECL_OR_STMT_LIST,
    INIT_DECL_LIST,
    DECL,
    FUNCTION_DECLARATOR,
    PARAMETER_LIST,
    PARAMETER_DECL,
    ARRAY_DECLARATOR,
    EXPRESSION_STATEMENT,
    LABELED_STATEMENT,
    COMPOUND_STATEMENT,
    POINTER_DECLARATOR,
    SIMPLE_DECLARATOR,
    IF_THEN,
    IF_THEN_ELSE,
    WHILE_STATEMENT,
    DO_STATEMENT,
    FOR_STATEMENT,
    BREAK_STATEMENT,
    CONTINUE_STATEMENT,
    RETURN_STATEMENT,
    GOTO_STATEMENT,
    NULL_STATEMENT,
    CONDITIONAL_EXPR,
    BINARY_EXPR,
    CAST_EXPR,
    UNARY_EXPR,
    PREFIX_EXPR,
    POSTFIX_EXPR,
    IDENTIFIER_EXPR,
    SUBSCRIPT_EXPR,
    FUNCTION_CALL,
    TYPE_NAME,
    TYPE_SPECIFIER,
    ABSTRACT_DECLARATOR,
    POINTER,
    DIR_ABS_DECL
};

typedef struct Node Node;

/* A Node, in addition to having links to its children, can contain data fields.
 * Examples:
 * - TYPE_SPECIFIER Node has a data field for what type it specifies, e.g. VOID
 * - UNARY_EXPRESSION Node has a data field for its operator, e.g. LOGICAL_NOT
 * MAX_ITEMS limits the number of data fields a Node may have.
 */
#define MAX_ITEMS 3


/* Because different Node types have a variety of data fields, when processing
 * a specific Node type we need a way to refer to fields specific to that type
 */
#define TYPE_SPEC 0
#define OPERATOR 0

    

/*
 * A Node may contain various types of data fields.
 */
union NodeData {
    /* fields with a symbolic (int or enum) representation */
    int symbols[MAX_ITEMS];
    /* Storage for literal data. Applies to identifiers and constants. */
    char ch;           /* CHAR_CONSTANT */
    unsigned long num; /* NUMBER_CONSTANT */
    char *str;         /* IDENTIFIER, STRING_CONSTANT */
};

/*
 * Node
 * The main data structure for the parse tree.
 */
struct Node {
    enum node_type n_type;
    union NodeData data;
    /* accommodate all node types, regardless of number of children */
    struct {
        Node *child1;
        Node *child2;
        Node *child3;
        Node *child4;
    } children;
};

#endif
