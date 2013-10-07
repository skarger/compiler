/* Nodes have a type corresponding to the grammar. Sometimes the type is
 * equivalent to a lexical token, but often the type has a higher-level
 * semantic value that the parser ascertains.
 */
enum node_type {
    TYPE_NAME,
    TYPE_SPECIFIER,
    POINTER
};

/* A given Node in the parse tree may have links to child Nodes */
/* MAX_CHILDREN allows enough links for any possible production */
#define MAX_CHILDREN 5

typedef int ChildIndex;

/* Common cases are for a Node to have one or two children
 * Based on the grammar of a given production, its children are naturally
 * thought of as left or right children. For example in this production:
    pointer : *
              * pointer
 *
 * When matching the second case we create a POINTER Node for the left asterisk,
 * and set that Node's RIGHT child to the result of parsing the right pointer.
 */
#define LEFT 0
#define RIGHT 1


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

/*
 * types that are not covered by token types
 * for example we have tokens for UNSIGNED, LONG, and INT
 * but we need a data type of UNSIGNED_LONG_INT.
 * Conversely we already have a VOID token so we do not need to copy it here.
 */
enum data_type {
    UNSIGNED_LONG_INT
};


/*
 * Storage for literal data. Applies to identifiers and constants.
 * IDENTIFIER, CHAR_CONSTANT, NUMBER_CONSTANT, STRING_CONSTANT
 */
union LiteralData {
    char ch;
    unsigned long num;
    char *str;
};

/* A Node may fields with a symbolic (int or enum) representation or
 * fields with literal data, e.g. a string value.
 */
union NodeData {
    int symbols[MAX_ITEMS];
    union LiteralData values;
};

/*
 * Node
 * The main data structure for the parse tree.
 */
struct Node {
    enum node_type n_type;
    union NodeData data;
    struct Node *children[MAX_CHILDREN];
};
typedef struct Node Node;

/*
 * Errors that are caught in the parsing step.
 */
enum parser_error {
    PE_SUCCESS = 0,
    PE_INVALID_DATA_TYPE = -1
};


/* function declarations */

/* tree traversal */
void pretty_print(Node *n);
void traverse_data_node(void *np);
void traverse_node(void *np);

/* node creation */
void *create_data_node(enum node_type, void *);
void *create_zero_item_node(enum node_type nt);
void *create_one_item_node(enum node_type nt, int item1);

/* helpers */
void *create_node(enum node_type nt); 
void append_child(Node *n, Node *child, ChildIndex chidx);
void initialize_children(Node *n);
char *get_type_name(enum data_type type);
void handle_parser_error(enum parser_error e, char *data, int line);
