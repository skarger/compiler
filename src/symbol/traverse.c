/* Functions for parse tree traversal. Perform processing including:
 *      symbol table generation
 */

#include <stdio.h>
#include "../include/traverse.h"
#include "../include/parse-tree.h"
#include "../../y.tab.h"

/*
 * start_traversal
 * Purpose: Kick off traversal of parse tree. Meant for parser to call.
 * Parameters:
 *  np      void * The node to start traversing from. Recursively traverses
 *          the children of np.
 * Returns: None
 * Side-effects: Allocates heap memory
 */
void start_traversal(void *np) {
    traversal_data * td;
    util_emalloc((void **) &td, sizeof(traversal_data));

    /* create container for each symbol table that will be created */
    /* pass the container across traversal so that new STs may be inserted */
    SymbolTableContainer *symbol_table_container = create_st_container();
    td->stc = symbol_table_container;
    traverse_node(np, td);
}

/*
 * traverse_node
 * Purpose: Recursively traverse nodes of parse tree.
 *          Perform processing at nodes, e.g. update symbol table.
 * Parameters:
 *  np      void * The node to start traversing from. Recursively traverses
 *          the children of np.
 *  td      traversal_data * symbol table related data
            to carry through traversals
 * Returns: None
 * Side-effects: None
 */
void traverse_node(void *np, traversal_data *td) {
    Node *n = (Node *) np;
    if (n == NULL) {
        return;
    }

    /* this node may or may not imply a scope transition */
    transition_scope(n, START);

    if (should_create_new_st()) {
        SymbolTable *st = create_symbol_table();
        insert_symbol_table(st, td->stc);

        #ifdef DEBUG
        fprintf(output, "/* node type: %s, "
            "creating a new symbol table at scope: %d */\n",
            get_node_name(n->n_type), get_scope());
        #endif
    }

    /*
    if is a decl
    note that we are processing a decl
    first item is type spec - look for it
    followed by 0 or more pointers
    followed by an identifier
    followed by array brackets or function param list
    */

    switch (n->n_type) {
        case FUNCTION_DEF_SPEC:
        case DECL:
            /* add_symbol(); */
            traverse_node(n->children.child1, td);
            traverse_node(n->children.child2, td);
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
            traverse_node(n->children.child1, td);
            traverse_node(n->children.child2, td);
            break;
        case EXPRESSION_STATEMENT:
        case COMPOUND_STATEMENT:
        case RETURN_STATEMENT:
        case GOTO_STATEMENT:
            traverse_node(n->children.child1, td);
            break;
        case IF_THEN:
        case IF_THEN_ELSE:
            traverse_conditional_statement(n, td);
            break;
        case WHILE_STATEMENT:
        case DO_STATEMENT:
        case FOR_STATEMENT:
            traverse_iterative_statement(n, td);
            break;
        case BREAK_STATEMENT:
        case CONTINUE_STATEMENT:
        case NULL_STATEMENT:
            break;
        case CONDITIONAL_EXPR:
            traverse_node(n->children.child1, td);
            traverse_node(n->children.child2, td);
            traverse_node(n->children.child3, td);
            break;
        case BINARY_EXPR:
            traverse_node(n->children.child1, td);
            /* n->data.attributes[OPERATOR] */
            traverse_node(n->children.child2, td);
            break;
        case TYPE_SPECIFIER:
            /* printf("ts %d\n", n->data.attributes[TYPE_SPEC]); */
            break;
        case POINTER:
            traverse_pointers(n, td);
            break;
        case UNARY_EXPR:
        case PREFIX_EXPR:
            /* n->data.attributes[OPERATOR] */
            traverse_node(n->children.child1, td);
            break;
        case POSTFIX_EXPR:
            traverse_node(n->children.child1, td);
            /* n->data.attributes[OPERATOR] */
            break;
        case SIMPLE_DECLARATOR:
        case IDENTIFIER_EXPR:
            break;
        case IDENTIFIER:
            /*
                if processing decl
                    add to type tree
                    check current symbol table for duplicate id
                    if duplicated
                        if it is a function def duplicating a func prototype or vice versa
                            check that num params and param types match
                        else
                            error
                else
                    look for id in symbol table and enclosing
                    if not found error
            */
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
void traverse_iterative_statement(void *np, traversal_data *td) {
    Node *n = (Node *) np;
    if (n == NULL) {
        return;
    }
    switch (n->n_type) {
        case WHILE_STATEMENT:
        case DO_STATEMENT:
            traverse_node(n->children.child1, td);
            traverse_node(n->children.child2, td);
            break;
        case FOR_STATEMENT:
            traverse_node(n->children.child1, td);
            traverse_node(n->children.child2, td);
            traverse_node(n->children.child3, td);
            traverse_node(n->children.child4, td);
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
void traverse_conditional_statement(void *np, traversal_data *td) {
    Node *n = (Node *) np;
    if (n == NULL) {
        return;
    }
    switch (n->n_type) {
        case IF_THEN:
            traverse_node(n->children.child1, td);
            traverse_node(n->children.child2, td);
            break;
        case IF_THEN_ELSE:
            traverse_node(n->children.child1, td);
            traverse_node(n->children.child2, td);
            traverse_node(n->children.child3, td);
            break;
        default:
            break;
    }
}

void traverse_pointers(void *np, traversal_data *td) {
    Node *n = (Node *) np;
    if (n == NULL || n->n_type != POINTER) {
        return;
    }
    do {
        n = n->children.child1;
    } while (n != NULL && n->n_type == POINTER);
}
