/* Nodes have a type corresponding to the grammar. Sometimes the type is
 * equivalent to a lexical token, but often the type has a higher-level
 * semantic value that the parser ascertains.
 */
enum node_type {
    TYPE_NAME,
    TYPE_SPECIFIER,
    POINTER,
    ABSTRACT_DECLARATOR,
    PAREN_DIR_ABS_DECL,
    BRACKET_DIR_ABS_DECL,
    BINARY_EXPR
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
#define BINARY_OP 0


/*
 * Depending on node type, a Node has various numbers of children.
 * They also have different names implied by the grammar.
 */
union NodeChildren {
    struct {
        Node *left;
        Node *right;
    } bin_expr;

    struct {
        Node *right;
    } ptr;

    struct {
        Node *type_spec;
        Node *abs_decl;
    } type_name;

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
void traverse_data_node(void *np);
void traverse_node(void *np);
void traverse_direct_abstract_declarator(Node *n);

/* node creation */
void *create_node(enum node_type nt, ...);
void *create_data_node(enum node_type, void *);
void *create_zero_item_node(enum node_type nt);
void *create_one_item_node(enum node_type nt, int item1);
void *create_binary_expr_node(int op, void *left, void *right);

/* helpers */
void *construct_node(enum node_type nt);
void initialize_children(Node *n);
char *get_type_name(enum data_type type);
char *get_operator_value(int op);
void handle_parser_error(enum parser_error e, char *data, int line);

