/*
 * header file for parser.y
 * In addition to function declarations, contains data definitions
 * for parse tree nodes.
 */



#define DEBUG
#undef  DEBUG

/* Nodes have a type corresponding to the grammar. Sometimes the type is
 * equivalent to a lexical token, but often the type has a higher-level
 * semantic value that the parser ascertains.
 */
enum node_type {
    FUNCTION_DEFINITION,
    TYPE_SPEC_DECL,
    DECL_OR_STMT_LIST,
    INIT_DECL_LIST,
    DECL,
    PAREN_DIR_DECL,
    FUNCTION_DECL,
    ARRAY_DECL,
    EXPRESSION_STATEMENT,
    LABELED_STATEMENT,
    COMPOUND_STATEMENT,
    POINTER_DECLARATOR,
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
    TYPE_SPEC_ABS_DECL,
    TYPE_SPECIFIER,
    ABSTRACT_DECLARATOR,
    POINTER,
    PAREN_DIR_ABS_DECL,
    BRACKET_DIR_ABS_DECL,
    UNARY_EXPR,
    PREFIX_EXPR,
    PAREN_EXPR,
    SUBSCRIPT_EXPR,
    FUNCTION_CALL,
    POSTFIX_INCREMENT,
    POSTFIX_DECREMENT
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
#define TYPE 0
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

/*
 * Errors that are caught in the parsing step.
 */
enum parser_error {
    PE_SUCCESS = 0,
    PE_INVALID_DATA_TYPE = -1,
    PE_UNRECOGNIZED_NODE_TYPE = -2,
    PE_UNRECOGNIZED_OP = -3
};


/* function declarations */

/* tree traversal */
void pretty_print(Node *n);
void traverse_node(void *np);
void print_data_node(void *np);
void traverse_direct_abstract_declarator(Node *n);
void traverse_conditional_statement(void *np);
void traverse_iterative_statement(void *np);

/* node creation */
void *create_node(enum node_type nt, ...);

/* helpers */
void set_literal_data(Node *n, YYSTYPE data);
void set_type(Node *n, int type_spec);
void set_operator(Node *n, int op);
int has_literal_data(enum node_type nt);
int has_operator(enum node_type nt);
int number_of_children(enum node_type nt);

void *construct_node(enum node_type nt);
void initialize_children(Node *n);
void append_children(Node *n, int num_children, ...);
char *get_type_spec(enum data_type type);
char *get_operator_value(int op);
void handle_parser_error(enum parser_error e, char *data, int line);
int parenthesize(enum node_type nt);
void print_pointers(Node *n);

