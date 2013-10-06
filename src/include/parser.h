enum node_type {
    TYPE_SPECIFIER
};

/*
 * types that are not covered by token types
 * for example we have an AMPERSAND token, but since it's overloaded we need
 * more specific meanings here that the parser will uncover
 * conversely we already have a VOID token so we do not need to copy it here
 */
enum data_type {
    POINTER
};


enum parser_error {
    PE_SUCCESS = 0,
    PE_INVALID_DATA_TYPE = -1
};

struct Node {
    enum node_type n_type;
    int type;
    /* left and right children */
    struct Node *left;
    struct Node *right;
};


union Data {
    char ch;
    unsigned long num;
    char *str;
};

/*
 * identifier: IDENTIFIER
 * constant: CHAR_CONSTANT, NUMBER_CONSTANT, STRING_CONSTANT
 */
struct DataNode {
    enum node_type n_type;
    union Data data;
};

typedef struct DataNode DataNode;
typedef struct Node Node;

void *create_type_spec_node(enum data_type type);
void pretty_print(Node *n);
char *get_type_name(enum data_type type);
void *create_data_node(enum node_type, void *);
void handle_parser_error(enum parser_error e, char *data, int line);
