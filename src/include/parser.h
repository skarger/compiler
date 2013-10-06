#define MAX_ITEMS 3

enum node_type {
    TYPE_SPECIFIER,
    POINTER
};

/* indices to Node items array */
#define TYPE 0

/*
 * types that are not covered by token types
 * for example we have an AMPERSAND token, but since it's overloaded we need
 * more specific meanings here that the parser will uncover
 * conversely we already have a VOID token so we do not need to copy it here
 */
enum data_type {
    X
};


enum parser_error {
    PE_SUCCESS = 0,
    PE_INVALID_DATA_TYPE = -1
};


/*
 * identifier: IDENTIFIER
 * constant: CHAR_CONSTANT, NUMBER_CONSTANT, STRING_CONSTANT
 */
union LiteralData {
    char ch;
    unsigned long num;
    char *str;
};

union NodeData {
    /* fields in node have a symbolic (int or enum) representation */
    int symbols[MAX_ITEMS];
    /* fields in node have literal data, e.g. a string value */
    union LiteralData values;
};

/* default Node has two children, although sometimes we won't need them */
struct Node {
    enum node_type n_type;
    union NodeData data;
    /* left and right children */
    struct Node *left;
    struct Node *right;
};




typedef struct Node Node;

void pretty_print(Node *n);
char *get_type_name(enum data_type type);
void *create_data_node(enum node_type, void *);
void handle_parser_error(enum parser_error e, char *data, int line);

void traverse_data_node(void *np);
void traverse_node(void *np);


void *create_zero_item_node(enum node_type nt);
void *create_one_item_node(enum node_type nt, int item1);


/* helpers */
void *create_node(enum node_type nt); 
void initialize_children(Node *n);

