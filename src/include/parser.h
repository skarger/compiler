#define DEBUG

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
 * Depending on node type, a Node has various numbers of children.
 * They also have different names implied by the grammar.
 */
union NodeChildren {
    struct {
        Node *func_def_spec;
        Node *cmpd_stmt;
    } func_def;

    struct {
        Node *type_spec;
        Node *decl;
    } type_spec_decl;

    struct {
        Node *decl_stmt_ls;
        Node *decl_stmt;
    } decl_stmt_ls;

    struct {
        Node *type_spec;
        Node *init_decl_ls;
    } decl;

    struct {
        Node *decl;
    } paren_dir_decl;

    struct {
        Node *dir_decl;
        Node *param_ls;
    } func_decl;

    struct {
        Node *dir_decl;
        Node *cond_expr;
    } array_decl;

    struct {
        Node *expr;
    } expr_stmt;

    struct {
        Node *expr;
    } return_stmt;

    struct {
        Node *id;
    } goto_stmt;

    struct {
        Node *label;
        Node *stmt;
    } lab_stmt;

    struct {
        Node *decl_or_stmt_ls;
    } cmpd_stmt;

    struct {
        Node *expr;
        Node *stmt;
    } while_do;

    struct {
        Node *init;
        Node *cond;
        Node *loop;
        Node *stmt;
    } for_stmt;

    struct {
        Node *init_decl_ls;
        Node *decl;
    } init_decl_ls;

    struct {
        Node *ptr;
        Node *dir_decl;
    } ptr_decl;

    struct {
        Node *cond;
        Node *val_if_true;
    } if_then;

    struct {
        Node *cond;
        Node *val_if_true;
        Node *val_if_false;
    } if_then_else;

    struct {
        Node *operand;
    } unary_op;

    struct {
        Node *left;
        Node *right;
    } bin_op;

    struct {
        Node *pstf_expr;
        Node *expr_list;
    } function_call;

    struct {
        Node *type_name;
        Node *cast_expr;
    } cast_expr;

    struct {
        Node *pstf_expr;
        Node *expr;
    } subs_expr;

    struct {
        Node *right;
    } ptr;

    struct {
        Node *type_spec;
        Node *abs_decl;
    } type_spec_abs_decl;

    struct {
        Node *ptr;
        Node *dir_abs_decl;
    } abs_decl;

    struct {
        Node *abs_decl;
        Node *dir_abs_decl;
        Node *cond_expr;
    } dir_abs_decl;
};

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
    union NodeChildren children;
};

/*
 * Errors that are caught in the parsing step.
 */
enum parser_error {
    PE_SUCCESS = 0,
    PE_INVALID_DATA_TYPE = -1,
    PE_UNRECOGNIZED_NODE_TYPE = -2
};


/* function declarations */

/* tree traversal */
void pretty_print(Node *n);
void traverse_node(void *np);
void traverse_data_node(void *np);
void traverse_direct_abstract_declarator(Node *n);
void traverse_conditional_statement(void *np);
void traverse_iterative_statement(void *np);

/* node creation */
void *create_node(enum node_type nt, ...);
void *create_data_node(enum node_type, void *);
void *create_zero_item_node(enum node_type nt);
void *create_one_item_node(enum node_type nt, int item1);

/* helpers */
void *construct_node(enum node_type nt);
void append_two_children(Node *n, Node *child1, Node *child2);
void append_three_children(Node *n, Node *child1, Node *child2, Node *child3);
void append_four_children(Node *n, Node *child1, Node *child2,
    Node *child3, Node *child4);
void initialize_children(Node *n);
char *get_type_spec(enum data_type type);
char *get_operator_value(int op);
void handle_parser_error(enum parser_error e, char *data, int line);
int parenthesize(enum node_type nt);
void print_pointers(Node *n);

