/*
 * header file for parser.y
 */

#include "parse-tree.h"

#define DEBUG
#undef  DEBUG




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

/* printing */
void pretty_print(void *np);
void print_data_node(void *np);
void print_direct_abstract_declarator(Node *n);
void print_conditional_statement(void *np);
void print_iterative_statement(void *np);
char *get_node_name(enum data_type nt);
char *get_operator_value(int op);
int parenthesize(enum data_type nt);
void print_pointers(Node *n);

/* node constructor, setters, and getters */
Node *create_node(int node_type, ...);
void *construct_node(enum data_type nt);
void initialize_children(Node *n);
void append_children(Node *n, int num_children, ...);
void set_literal_data(Node *n, YYSTYPE data);
void set_type(Node *n, int type_spec);
void set_operator(Node *n, int op);
void set_node_type(Node *n, enum data_type nt);
int has_literal_data(enum data_type nt);
int has_operator(enum data_type nt);
int number_of_children(enum data_type nt);
void set_symbol_table_entry(Node *n, Symbol *s);

/* error handling */
void handle_parser_error(enum parser_error e, char *data, int line);


