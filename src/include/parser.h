enum node_type {
    TYPE_SPECIFIER
};

enum data_type {
    VOID_T,
    POINTER_T
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
