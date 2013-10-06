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

struct Node {
    enum node_type n_type;
    int type;
    /* left and right children */
    struct Node *left;
    struct Node *right;
};

typedef struct Node Node;

void *create_type_spec_node(enum data_type type);
void pretty_print(Node *n);
char *get_type_name(enum data_type type);
