/* Functions for parse tree traversal. Perform processing including:
 *      symbol table generation
 */

#include <stdio.h>
#include "../include/traverse.h"
#include "../include/parse-tree.h"
#include "../../y.tab.h"

static traversal_data * td = NULL;


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
        util_emalloc((void **) &td, sizeof(traversal_data));
        /* create container for each symbol table that will be created */
        /* pass the container across traversal so that new STs may be inserted */
        SymbolTableContainer *symbol_table_container = create_st_container();
        td->stc = symbol_table_container;
        td->current_base_type = NO_DATA_TYPE;
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
 *  td      traversal_data * symbol table related data
            to carry through traversals
 * Returns: None
 * Side-effects: None
 */
void traverse_node(Node *n, traversal_data *td) {
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
            print_symbol_table(td->outfile, td->stc->current_st);
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
            /* error if it cannot be resolved */
            /* do this before processing first child because first child */
            /* will reset the current symbol when it is resolved */
            traverse_node(n->children.child2, td);
            set_symbol_array_size(td->current_symbol, 0);

            /* first child: direct declarator */
            /* should ultimately lead to a simple declarator */
            traverse_node(n->children.child1, td);
            break;
        case ABSTRACT_DECLARATOR:
        case DIR_ABS_DECL:
        case FUNCTION_DEFINITION:
        case PARAMETER_DECL:
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

void traverse_pointers(Node *n, traversal_data *td) {
    /* push pointers onto type tree */
    while (n != NULL && n->n_type == POINTER) {
        push_symbol_type(td->current_symbol, POINTER);
        n = n->children.child1;
    }
}

void create_symbol_if_necessary(traversal_data *td) {
    if (td->current_symbol == NULL) {
        Symbol *s = create_symbol();
        push_symbol_type(s, td->current_base_type);
        td->current_symbol = s;
    }
}

void reset_current_symbol(traversal_data *td) {
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
