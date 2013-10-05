enum node_type {
    TYPE_SPECIFIER
};

typedef struct {
    enum node_type n_type;
    int type;
} Node;

void *create_type_spec_node(int type);
void pretty_print(Node *n);
char *get_type_name(int token);
