/*
 * header file for parser.y
 */

#include "parse-tree.h"

#define DEBUG
#undef  DEBUG
#define DEBUG



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
void set_node_type(Node *n, enum node_type nt);
int has_literal_data(enum node_type nt);
int has_operator(enum node_type nt);
int number_of_children(enum node_type nt);

void *construct_node(enum node_type nt);
void initialize_children(Node *n);
void append_children(Node *n, int num_children, ...);
char *get_type_spec(enum data_type type);
char *get_node_name(enum node_type nt);
char *get_operator_value(int op);
void handle_parser_error(enum parser_error e, char *data, int line);
int parenthesize(enum node_type nt);
void print_pointers(Node *n);

