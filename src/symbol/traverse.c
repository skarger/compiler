/* Functions for parse tree traversal. Perform processing including:
 *      symbol table generation
 */

#include <stdio.h>
#include "../include/traverse.h"
#include "../include/parse-tree.h"
#include "../include/utilities.h"
#include "../include/literal.h"
#include "../../y.tab.h"

/* external variable for traversal data. this is the defining declaration. */
TraversalData *td = NULL;

/* file helper procs */
enum Boolean invalid_operand(long operand);
enum Boolean array_bound_optional(TraversalData *td);

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
    FILE *output = stdout;
    if (td == NULL) {
        util_emalloc((void **) &td, sizeof(TraversalData));
        /* create container for each symbol table that will be created */
        /* pass the container across traversal so that new STs may be inserted */
        SymbolTableContainer *symbol_table_container = create_st_container();
        td->stc = symbol_table_container;
        td->current_base_type = NO_DATA_TYPE;
        td->processing_parameter_decl = FALSE;
        td->outfile = output;
    }
    traverse_node(np, td);
}

/*
 * traverse_node
 * Purpose: Recursively traverse nodes of parse tree.
 *          Perform processing at nodes, e.g. update symbol table.
 * Parameters:
 *  np      Node * The node to start traversing from. Recursively traverses
 *          the children of np.
 *  td      TraversalData * symbol table related data
            to carry through traversals
 * Returns: None
 * Side-effects: None
 */
void traverse_node(Node *n, TraversalData *td) {
    if (n == NULL) {
        return;
    }

    unsigned long array_size;

    /* this node may or may not imply a scope transition */
    transition_scope(n, START);

    if (should_create_new_st()) {
        SymbolTable *st = create_symbol_table();
        insert_symbol_table(st, td->stc);
    }

    switch (n->n_type) {
        case FUNCTION_DEF_SPEC:
            /* type_specifier */
            /* create type tree */
            traverse_node(n->children.child1, td);
            /* pointer_declarator or function_declarator */
            /* pointer_declarator: append to return type */
            /* function declarator: traverse direct_declarator and param list */
            /* process params */
            traverse_node(n->children.child2, td);
            break;
        case DECL:
            /* first child is a type_specifier */
            /* save it because there might be multiple declarators in child2 */
            td->current_base_type = 
                n->children.child1->data.attributes[TYPE_SPEC];
            traverse_node(n->children.child2, td);
            break;
        case INIT_DECL_LIST:
            /* we will create a symbol for each declarator we recurse into */
            /* child1 may itself be a list of declarators or just one */
            /* we do know that child2 is a single declarator */
            traverse_node(n->children.child1, td);
            traverse_node(n->children.child2, td);
            break;
        case SIMPLE_DECLARATOR:
            /* here we have the actual identifier for the symbol table entry */
            /* first create a symbol if it does not already exist */
            /* set its name */
            /* the symbol should have all data, so put it in the table */
            /* reset */
            create_symbol_if_necessary(td);
            set_symbol_name(td->current_symbol, n->data.str);
            append_symbol(td->stc->current_st, td->current_symbol);
            reset_current_symbol(td);
            break;
        case POINTER_DECLARATOR:
            create_symbol_if_necessary(td);
            /* pointer(s) */
            traverse_node(n->children.child1, td);
            /* direct declarator */
            traverse_node(n->children.child2, td);
            break;
        case POINTER:
            traverse_pointers(n, td);
            break;
        case FUNCTION_DECLARATOR:
            traverse_node(n->children.child1, td);
            traverse_node(n->children.child2, td);
            break;
        case ARRAY_DECLARATOR:
            create_symbol_if_necessary(td);
            push_symbol_type(td->current_symbol, ARRAY);
            /* second child: constant expr, i.e. conditional expr */
            /* need to resolve this to determine array size */
            /* do this before processing first child because first child */
            /* will reset the current symbol when it is resolved */
            if (n->children.child2 != NULL) {
                array_size = resolve_constant_expr(n->children.child2);
                /* TODO: support unsigned long array bounds */
                /* this will incorrectly error for unsigned longs > LONG_MAX */
                if (array_size == VARIABLE_VALUE) {
                    handle_symbol_error(STE_VARIABLE_ARRAY_SIZE, "array bound");
                    array_size = UNSPECIFIED_VALUE;
                } else if (array_size == NON_INTEGRAL_VALUE) {
                    handle_symbol_error(STE_ARRAY_SIZE_TYPE, "array bound");
                    array_size = UNSPECIFIED_VALUE;
                } else if ((signed long) array_size <= 0) {
                    char *err = util_compose_numeric_message("array bound %ld",
                                                            array_size);
                    handle_symbol_error(STE_NON_POSITIVE_ARRAY_SIZE, err);
                    array_size = UNSPECIFIED_VALUE;
                }
                set_symbol_array_size(td->current_symbol, array_size);
            } else {
                if (!array_bound_optional(td)) {
                    handle_symbol_error(STE_ARRAY_SIZE_MISSING,
                                    "array type has incomplete element type");
                }
                set_symbol_array_size(td->current_symbol, UNSPECIFIED_VALUE);
            }
            /* first child: direct declarator */
            /* should ultimately lead to a simple declarator */
            traverse_node(n->children.child1, td);
            break;
        case PARAMETER_DECL:
            td->processing_parameter_decl = TRUE;
            traverse_node(n->children.child1, td);
            traverse_node(n->children.child2, td);
            td->processing_parameter_decl = FALSE;
            break;
        case ABSTRACT_DECLARATOR:
        case DIR_ABS_DECL:
        case FUNCTION_DEFINITION:
        case CAST_EXPR:
        case TYPE_NAME:
        case DECL_OR_STMT_LIST:
        case PARAMETER_LIST:
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
        case UNARY_EXPR:
        case PREFIX_EXPR:
            /* n->data.attributes[OPERATOR] */
            traverse_node(n->children.child1, td);
            break;
        case POSTFIX_EXPR:
            traverse_node(n->children.child1, td);
            /* n->data.attributes[OPERATOR] */
            break;
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
void traverse_iterative_statement(void *np, TraversalData *td) {
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
void traverse_conditional_statement(void *np, TraversalData *td) {
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

void traverse_pointers(Node *n, TraversalData *td) {
    /* push pointers onto type tree */
    while (n != NULL && n->n_type == POINTER) {
        push_symbol_type(td->current_symbol, POINTER);
        n = n->children.child1;
    }
}

unsigned long resolve_constant_expr(Node *n) {
    unsigned long resolve_conditional_expr(Node *n);
    unsigned long resolve_binary_expr(Node *n);
    unsigned long resolve_unary_expr(Node *n);

    /* error if it cannot be resolved */
    switch (n->n_type) {
        /* base cases */
        case CHAR_CONSTANT:
            return n->data.ch;
        case NUMBER_CONSTANT:
            return n->data.num;
        /* recursive cases */
        case CONDITIONAL_EXPR:
            return resolve_conditional_expr(n);
        case BINARY_EXPR:
            return resolve_binary_expr(n);
        case UNARY_EXPR:
            return resolve_unary_expr(n);
        /* error cases */
        case PREFIX_EXPR:
        case POSTFIX_EXPR:
        case SUBSCRIPT_EXPR:
        case FUNCTION_CALL:
        case IDENTIFIER_EXPR:
            return VARIABLE_VALUE;
        case STRING_CONSTANT:
            return NON_INTEGRAL_VALUE;
        default:
            return UNSPECIFIED_VALUE;
    }
}

enum Boolean invalid_operand(long operand) {
    switch (operand) {
        case NON_INTEGRAL_VALUE:
        case VARIABLE_VALUE:
        case UNSPECIFIED_VALUE:
            return TRUE;
        default:
            return FALSE;
    }
}

/* helpers for resolve constant expr */
unsigned long resolve_conditional_expr(Node *n) {
    unsigned long child1, child2, child3;
    if (invalid_operand(child1)) {
        return child1;
    }
    if (invalid_operand(child2)) {
        return child2;
    }
    if (invalid_operand(child3)) {
        return child3;
    }
    child1 = resolve_constant_expr(n->children.child1);
    child2 = resolve_constant_expr(n->children.child2);
    child3 = resolve_constant_expr(n->children.child3);
    return child1 ? child2 : child3;
}

unsigned long resolve_binary_expr(Node *n) {
    unsigned long child1, child2;
    child1 = resolve_constant_expr(n->children.child1);
    child2 = resolve_constant_expr(n->children.child2);
    if (invalid_operand(child1)) {
        return child1;
    }
    if (invalid_operand(child2)) {
        return child2;
    }
    switch (n->data.attributes[OPERATOR]) {
        case LOGICAL_OR:
            return child1 || child2;
        case LOGICAL_AND:
            return child1 && child2;
        case BITWISE_OR:
            return child1 | child2;
        case BITWISE_XOR:
            return child1 ^ child2;
        case AMPERSAND:
            return child1 & child2;
        case EQUAL:
            return child1 == child2;
        case NOT_EQUAL:
            return child1 != child2;
        case LESS_THAN:
            return child1 < child2;
        case LESS_THAN_EQUAL:
            return child1 <= child2;
        case GREATER_THAN:
            return child1 > child2;
        case GREATER_THAN_EQUAL:
            return child1 >= child2;
        case BITWISE_LSHIFT:
            return child1 << child2;
        case BITWISE_RSHIFT:
            return child1 >> child2;
        case PLUS:
            return child1 + child2;
        case MINUS:
            return child1 - child2;
        case ASTERISK:
            return child1 * child2;
        case DIVIDE:
            return child1 / child2;
        case REMAINDER:
            return child1 % child2;
        /* value of assignment and comma expression is the value of the RHS */
        case ASSIGN:
        case ADD_ASSIGN:
        case SUBTRACT_ASSIGN:
        case MULTIPLY_ASSIGN:
        case DIVIDE_ASSIGN:
        case REMAINDER_ASSIGN:
        case BITWISE_LSHIFT_ASSIGN:
        case BITWISE_RSHIFT_ASSIGN:
        case BITWISE_AND_ASSIGN:
        case BITWISE_XOR_ASSSIGN:
        case BITWISE_OR_ASSIGN:
        case COMMA:
            return VARIABLE_VALUE;
        default:
            /* error */
            return VARIABLE_VALUE;
    }
}

unsigned long resolve_unary_expr(Node *n) {
    unsigned long child1;
    child1 = resolve_constant_expr(n->children.child1);
    if (invalid_operand(child1)) {
        return child1;
    }
    switch (n->data.attributes[OPERATOR]) {
        case MINUS:
            return -child1;
        case PLUS:
            return +child1;
        case LOGICAL_NOT:
            return !child1;
        case BITWISE_NOT:
            return ~child1;
        case AMPERSAND:
            return NON_INTEGRAL_VALUE;
        case ASTERISK:
            return VARIABLE_VALUE;
        default:
            return UNSPECIFIED_VALUE;
    }
}

enum Boolean array_bound_optional(TraversalData *td) {
    return (td->processing_parameter_decl &&
            all_array_bounds_specified(td->current_symbol));
}

void create_symbol_if_necessary(TraversalData *td) {
    if (td->current_symbol == NULL) {
        Symbol *s = create_symbol();
        push_symbol_type(s, td->current_base_type);
        td->current_symbol = s;
    }
}

void reset_current_symbol(TraversalData *td) {
    td->current_symbol = NULL;
}

void print_symbol(FILE *out, Symbol *s) {
    fprintf(out, "/*\n");
    fprintf(out, " * symbol: %s\n", get_symbol_name(s));
    fprintf(out, " * type: %s\n", symbol_type_string(s));
    /* if function: function params */
    fprintf(out, " */\n");
}

void print_symbol_table(FILE *out, SymbolTable *st) {
    fprintf(out, "/*\n");
    fprintf(out, " * symbol table\n");
    fprintf(out, " * scope: %s\n", get_st_scope(st));
    fprintf(out, " * overloading class: %s\n", get_st_overloading_class(st));
    fprintf(out, " */\n");
    Symbol *cur = get_st_symbols(st);
    while (cur != NULL) {
        print_symbol(out, cur);
        cur = cur->next;
    }
}

