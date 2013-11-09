/* Functions for parse tree traversal. Perform processing including:
 *      symbol table generation
 */

#include <stdio.h>
#include "../include/traverse.h"
#include "../include/parse-tree.h"

#include "../include/symbol.h"
#include "../../y.tab.h"

/*
 * traverse_node
 * Purpose: Recursively traverse nodes of parse tree.
 *          Perform processing at nodes, e.g. update symbol table.
 * Parameters:
 *  np      void * The node to start traversing from. Recursively traverses
 *          the children of np.
 * Returns: None
 * Side-effects: None
 */
void traverse_node(void *np) {
    Node *n = (Node *) np;
    if (n == NULL) {
        return;
    }
    /* printf("traverse_node: node type: %s\n", get_node_name(n->n_type)); */

    /* this node may or may not imply a scope transition */
    transition_scope(n, START);
    printf("nt: %s, b %d", get_node_name(n->n_type), should_create_new_st());
    if (should_create_new_st()) {
        printf("should create new st\n", get_node_name(n->n_type));
        create_symbol_table();
    }

    switch (n->n_type) {
        case FUNCTION_DEF_SPEC:
        case DECL:
            /* add_symbol(); */
            traverse_node(n->children.child1);
            traverse_node(n->children.child2);
            break;
        case DIR_ABS_DECL:
        case ABSTRACT_DECLARATOR:
        case POINTER_DECLARATOR:
        case FUNCTION_DEFINITION:
        case PARAMETER_DECL:
        case CAST_EXPR:
        case TYPE_NAME:
        case DECL_OR_STMT_LIST:
        case PARAMETER_LIST:
        case INIT_DECL_LIST:
        case FUNCTION_DECLARATOR:
        case ARRAY_DECLARATOR:
        case LABELED_STATEMENT:
        case SUBSCRIPT_EXPR:
        case FUNCTION_CALL:
            traverse_node(n->children.child1);
            traverse_node(n->children.child2);
            break;
        case EXPRESSION_STATEMENT:
        case COMPOUND_STATEMENT:
        case RETURN_STATEMENT:
        case GOTO_STATEMENT:
            traverse_node(n->children.child1);
            break;
        case IF_THEN:
        case IF_THEN_ELSE:
            traverse_conditional_statement(n);
            break;
        case WHILE_STATEMENT:
        case DO_STATEMENT:
        case FOR_STATEMENT:
            traverse_iterative_statement(n);
            break;
        case BREAK_STATEMENT:
        case CONTINUE_STATEMENT:
        case NULL_STATEMENT:
            break;
        case CONDITIONAL_EXPR:
            traverse_node(n->children.child1);
            traverse_node(n->children.child2);
            traverse_node(n->children.child3);
            break;
        case BINARY_EXPR:
            traverse_node(n->children.child1);
            /* n->data.symbols[OPERATOR] */
            traverse_node(n->children.child2);
            break;
        case TYPE_SPECIFIER:
            /* printf("ts %d\n", n->data.symbols[TYPE_SPEC]); */
            break;
        case POINTER:
            traverse_pointers(n);
            break;
        case UNARY_EXPR:
        case PREFIX_EXPR:
            /* n->data.symbols[OPERATOR] */
            traverse_node(n->children.child1);
            break;
        case POSTFIX_EXPR:
            traverse_node(n->children.child1);
            /* n->data.symbols[OPERATOR] */
            break;
        case SIMPLE_DECLARATOR:
        case IDENTIFIER_EXPR:
        case IDENTIFIER:
            /* symbol table entry */
            break;
        case CHAR_CONSTANT:
        case NUMBER_CONSTANT:
        case STRING_CONSTANT:
            break;
        default:
            break;
    }
    transition_scope(n, END);

}

/*
 * traverse_iterative_statement
 * Purpose: Helper function for traverse_node.
 *          Traverses iterative statements.
 * Parameters:
 *  np      void * The node to start traversing from. Recursively traverses
 *          the children of np.
 * Returns: None
 * Side-effects: None
 */
void traverse_iterative_statement(void *np) {
    Node *n = (Node *) np;
    if (n == NULL) {
        return;
    }
    switch (n->n_type) {
        case WHILE_STATEMENT:
        case DO_STATEMENT:
            traverse_node(n->children.child1);
            traverse_node(n->children.child2);
            break;
        case FOR_STATEMENT:
            traverse_node(n->children.child1);
            traverse_node(n->children.child2);
            traverse_node(n->children.child3);
            traverse_node(n->children.child4);
            break;
        default:
            break;

    }
}

/*
 * traverse_conditional_statement
 * Purpose: Helper function for traverse_node.
 *          Traverses conditional statements.
 * Parameters:
 *  np      void * The node to start traversing from. Recursively traverses
 *          the children of np.
 * Returns: None
 * Side-effects: None
 */
void traverse_conditional_statement(void *np) {
    Node *n = (Node *) np;
    if (n == NULL) {
        return;
    }
    switch (n->n_type) {
        case IF_THEN:
            traverse_node(n->children.child1);
            traverse_node(n->children.child2);
            break;
        case IF_THEN_ELSE:
            traverse_node(n->children.child1);
            traverse_node(n->children.child2);
            traverse_node(n->children.child3);
            break;
        default:
            break;
    }
}

void traverse_pointers(void *np) {
    Node *n = (Node *) np;
    if (n == NULL || n->n_type != POINTER) {
        return;
    }
    do {
        n = n->children.child1;
    } while (n != NULL && n->n_type == POINTER);
}
