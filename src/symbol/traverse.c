/* Functions for parse tree traversal. Perform processing including:
 *      symbol table generation
 */

#include <stdio.h>
#include "../include/traverse.h"
#include "../include/parse-tree.h"
#include "../include/symbol-utils.h"
#include "../include/scope-fsm.h"
#include "../include/utilities.h"
#include "../include/literal.h"
#include "../../y.tab.h"

/* external variable for traversal data. this is the defining declaration. */
TraversalData *td = NULL;

/* file helper procs */
void initialize_traversal_data(TraversalData *td);
long resolve_array_size(TraversalData *td, Node *n);
Boolean array_bound_optional(TraversalData *td);
Boolean invalid_operand(long operand);

/*
 * start_traversal
 * Purpose: Kick off traversal of parse tree. Meant for parser to call.
 * Parameters:
 *  n       Node * The node to start traversing from. Recursively traverses
 *          the children of n.
 * Returns: None
 * Side-effects: Allocates heap memory
 */
void start_traversal(Node *n) {
    FILE *output = stdout;
    if (td == NULL) {
        util_emalloc((void **) &td, sizeof(TraversalData));
        initialize_traversal_data(td);
        td->outfile = output;
    }
    traverse_node(n, td);
}

void initialize_traversal_data(TraversalData *td) {
    td->current_base_type = NO_DATA_TYPE;
    td->current_symbol = NULL;
    td->current_param_list = NULL;
    td->dummy_symbol = create_symbol();
    td->processing_parameters = FALSE;
    td->function_def_spec = FALSE;
    td->function_prototype = FALSE;
    /* create container for each symbol table that will be created */
    /* pass the container across traversal so that STs may be inserted */
    SymbolTableContainer *symbol_table_container = create_st_container();
    td->stc = symbol_table_container;
}

/*
 * traverse_node
 * Purpose: Recursively traverse nodes of parse tree.
 *          Perform processing at nodes, e.g. update symbol table.
 * Parameters:
 *  n       Node * The node to start traversing from. Recursively traverses
 *          the children of n.
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
    SymbolTable *enclosing;
    SymbolTable *function_other_names_st, *function_statement_labels_st;
    Symbol* function_symbol;
    Symbol *id_symbol;
    enum data_type decl_base_type;

    /* this node may or may not imply a scope transition */
    transition_scope(n, START, td->stc);

    switch (n->n_type) {
        case IDENTIFIER:
        case IDENTIFIER_EXPR:
            id_symbol = find_symbol(get_current_st(td->stc), n->data.str);
            if (id_symbol != NULL) {
                set_symbol_table_entry(n, id_symbol);
            } else {
                handle_symbol_error(STE_ID_UNDECLARED, n->data.str);
                set_symbol_table_entry(n, td->dummy_symbol);
            }
            break;
        case FUNCTION_DEFINITION:
            /* create the function level symbol tables */
            function_other_names_st =
                new_current_st(FUNCTION_SCOPE, OTHER_NAMES, td->stc);
            function_statement_labels_st =
                new_current_st(FUNCTION_SCOPE, STATEMENT_LABELS, td->stc);
            /* first child: function def spec */
            traverse_node(n->children.child1, td);
            /* second child: compound statement */
            /* traversing function def spec returned us to the file level ST */
            /* now switch back to function body ST */
            set_current_st(function_other_names_st, td->stc);
            traverse_node(n->children.child2, td);
            break;
        case FUNCTION_DEF_SPEC:
            td->function_def_spec = TRUE;
            traverse_node(n->children.child1, td);
            traverse_node(n->children.child2, td);
            td->function_def_spec = FALSE;
            break;
        case INIT_DECL_LIST:
            /* first child: initialized declarator list */
            /* save the base type of the decl */
            /* we may encounter a function declarator whose parameters */
            /* will overwrite the current base type, but we'll need it again */
            decl_base_type = td->current_base_type;
            traverse_node(n->children.child1, td);
            /* second child: initialized declarator */
            /* restore the base type of the decl */
            td->current_base_type = decl_base_type;
            traverse_node(n->children.child2, td);
            break;
        case PARAMETER_DECL:
            td->current_param_list =
                push_function_parameter(td->current_param_list);
            /* type specifier */
            traverse_node(n->children.child1, td);
            /* declarator */
            traverse_node(n->children.child2, td);
            /* this reset is redundant unless we had an abstract declarator */
            reset_current_symbol(td);
            break;
        case TYPE_SPECIFIER:
            td->current_base_type = n->data.attributes[TYPE_SPEC];
            if (td->processing_parameters) {
                push_parameter_type(td->current_param_list,
                                    n->data.attributes[TYPE_SPEC]);
            }
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
            if (!(td->function_def_spec)) {
                td->function_prototype = TRUE;
            }
            if (td->processing_parameters) {
                handle_symbol_error(STE_FUNCTION_POINTER, "function parameter");
            } else {
                create_symbol_if_necessary(td);
                if (symbol_outer_type(td->current_symbol) == ARRAY) {
                    handle_symbol_error(STE_FUNC_RET_ARRAY, "function decl");
                } else if (symbol_outer_type(td->current_symbol) == FUNCTION) {
                    handle_symbol_error(STE_FUNC_RET_FUNC, "function decl");
                }
                /* save the nascent function symbol then dive into parameters */
                function_symbol = td->current_symbol;
                reset_current_symbol(td);

                /* second child: parameters */
                td->processing_parameters = TRUE;
                traverse_node(n->children.child2, td);
                td->processing_parameters = FALSE;
                if (td->function_def_spec) {
                    /* return back to the file level ST to process it */
                    /* only for func defn since prototype has no body ST */
                    enclosing = td->stc->current_st[OTHER_NAMES]->enclosing;
                    set_current_st(enclosing, td->stc);
                }
                /* first child: direct declarator */
                /* resurrect the function symbol */
                td->current_symbol = function_symbol;
                push_symbol_type(td->current_symbol, FUNCTION);
                set_symbol_func_params(td->current_symbol,
                                        td->current_param_list);
                td->current_param_list = NULL;
                traverse_node(n->children.child1, td);
            }
            td->function_prototype = FALSE;
            break;
        case PARAMETER_LIST:
            /* second child: parameter decl */
            /* process the second child first so that we can push each */
            /* one onto the front and have them in order at the end */
            traverse_node(n->children.child2, td);
            /* first child: parameter list or parameter_decl */
            traverse_node(n->children.child1, td);
            break;
        case COMPOUND_STATEMENT:
            /* verify that we have an inner block to avoid creating an extra */
            /* ST for function blocks after we created it for the parameters */
            if (is_inner_block(td->stc->current_scope)) {
                new_current_st(td->stc->current_scope, OTHER_NAMES, td->stc);
            }
            traverse_node(n->children.child1, td);
            /* return to the STs at the enclosing scope */
            enclosing = td->stc->current_st[OTHER_NAMES]->enclosing;
            set_current_st(enclosing, td->stc);
            /* if we are not at an inner block then we are exiting a function */
            if (!is_inner_block(td->stc->current_scope)) {
                validate_statement_labels(td);
                enclosing = td->stc->current_st[STATEMENT_LABELS]->enclosing;
                set_current_st(enclosing, td->stc);
            }
            break;
        case ARRAY_DECLARATOR:
            create_symbol_if_necessary(td);
            if (symbol_outer_type(td->current_symbol) == FUNCTION) {
                handle_symbol_error(STE_ARRAY_OF_FUNC, "array declarator");
            }
            if (td->processing_parameters) {
                /* intentionally converting type from ARRAY to POINTER */
                push_parameter_type(td->current_param_list, POINTER);
                push_symbol_type(td->current_symbol, POINTER);
                /* array size is irrelevant but it may have nested symbols */
                array_size = resolve_array_size(td, n->children.child2);
            } else {
                push_symbol_type(td->current_symbol, ARRAY);
                /* second child: constant expr */
                /* need to resolve this to determine array size */
                /* do this before processing first child since we will */
                /* reset the current symbol after the first child is resolved */
                array_size = resolve_array_size(td, n->children.child2);
                set_symbol_array_size(td->current_symbol, array_size);
            }
            /* first child: direct declarator */
            traverse_node(n->children.child1, td);
            break;
        case SIMPLE_DECLARATOR:
            /* have an identifier */
            create_symbol_if_necessary(td);
            set_symbol_name(td->current_symbol, n->data.str);
            if (td->processing_parameters) {
                set_parameter_name(td->current_param_list, n->data.str);
                record_current_symbol(td, n);
            } else {
                record_current_symbol(td, n);
            }
            reset_current_symbol(td);
            break;
        case ABSTRACT_DECLARATOR:
            if (td->function_def_spec && td->processing_parameters) {
                handle_symbol_error(STE_ABS_DECL_PARAM, "abstract declarator");
            }
            create_symbol_if_necessary(td);
            traverse_node(n->children.child1, td);
            break;
        case DIR_ABS_DECL:
            /* first child: direct_abstract_declarator */
            traverse_node(n->children.child1, td);
            if (td->processing_parameters) {
                /* intentionally converting type from ARRAY to POINTER */
                push_parameter_type(td->current_param_list, POINTER);
            }
            /* second child: constant_expr */
            array_size = resolve_array_size(td, n->children.child2);
            break;
        case NAMED_LABEL:
            id_symbol = find_symbol(get_current_st(td->stc), n->data.str);
            if (id_symbol != NULL) {
                set_symbol_table_entry(n, id_symbol);
            } else {
                td->current_base_type = NAMED_LABEL;
                create_symbol_if_necessary(td);
                set_symbol_name(td->current_symbol, n->data.str);
                record_current_symbol(td, n);
                reset_current_symbol(td);
            }
            break;
        case LABELED_STATEMENT:
            traverse_node(n->children.child1, td);
            id_symbol = find_symbol(td->stc->current_st[STATEMENT_LABELS],
                                    n->children.child1->data.str);
            set_label_defined(id_symbol, TRUE);
            traverse_node(n->children.child2, td);
            break;
        /* nodes we simply pass through with respect to symbol table */
        case CONDITIONAL_EXPR:
            traverse_node(n->children.child1, td);
            traverse_node(n->children.child2, td);
            traverse_node(n->children.child3, td);
            break;
        case DECL:
        case PTR_ABS_DECL:
        case DECL_OR_STMT_LIST:
        case CAST_EXPR:
        case TYPE_NAME:
        case BINARY_EXPR:
        case SUBSCRIPT_EXPR:
        case FUNCTION_CALL:
        case POSTFIX_EXPR:
            traverse_node(n->children.child1, td);
            traverse_node(n->children.child2, td);
            break;
        case EXPRESSION_STATEMENT:
        case RETURN_STATEMENT:
        case GOTO_STATEMENT:
        case UNARY_EXPR:
        case PREFIX_EXPR:
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
        case CHAR_CONSTANT:
        case NUMBER_CONSTANT:
        case STRING_CONSTANT:
            break;
        default:
            break;
    }

    transition_scope(n, END, td->stc);
}

/*
 * traverse_iterative_statement
 * Purpose: Helper function for traverse_node.
 *          Traverses iterative statements.
 * Parameters:
 *  n       Node * The node to start traversing from. Recursively traverses
 *          the children of n.
 * Returns: None
 * Side-effects: None
 */
void traverse_iterative_statement(Node *n, TraversalData *td) {
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
 *  n       Node * The node to start traversing from. Recursively traverses
 *          the children of n.
 * Returns: None
 * Side-effects: None
 */
void traverse_conditional_statement(Node *n, TraversalData *td) {
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
        if (td->processing_parameters) {
            push_parameter_type(td->current_param_list, POINTER);
        }
        n = n->children.child1;
    }
}

/*
 * resolve_array_size
 * Purpose:
 *      Resolve a constant expression as an array bound into an integer value
 * Parameters:
 *      td: pointer to the the running traversal data structure, which will
 *          be inspected to determine if array bound is required
 *      n:  the Node * representing the constant expression
 * Returns:
 *      The size of the array as a long int.
 *      NOTE: will incorrectly error for unsigned long array bounds > LONG_MAX
 * Side Effects:
 *      Will emit errors for various unacceptable array bound specifiers
 */
long resolve_array_size(TraversalData *td, Node *n) {
    unsigned long array_size;
    if (n != NULL) {
        array_size = resolve_constant_expr(n);
        if (array_size == VARIABLE_VALUE) {
            handle_symbol_error(STE_VARIABLE_ARRAY_SIZE, "array bound");
            array_size = UNSPECIFIED_VALUE;
        } else if (array_size == NON_INTEGRAL_VALUE) {
            handle_symbol_error(STE_ARRAY_SIZE_TYPE, "array bound");
            array_size = UNSPECIFIED_VALUE;
        } else if (array_size == CAST_VALUE) {
            handle_symbol_error(STE_CAST_ARRAY_SIZE, "array bound");
            array_size = UNSPECIFIED_VALUE;
        } else if ((signed long) array_size <= 0) {
            char *err = util_compose_numeric_message("array bound %ld",
                                                    array_size);
            handle_symbol_error(STE_NON_POSITIVE_ARRAY_SIZE, err);
            array_size = UNSPECIFIED_VALUE;
        }
    } else {
        if (!array_bound_optional(td)) {
            handle_symbol_error(STE_ARRAY_SIZE_MISSING,
                            "array type has incomplete element type");
        }
        array_size = UNSPECIFIED_VALUE;
    }
    return (signed long) array_size;
}

Boolean array_bound_optional(TraversalData *td) {
    return (td->processing_parameters &&
            all_array_bounds_specified(td->current_symbol));
}

unsigned long resolve_constant_expr(Node *n) {
    unsigned long resolve_conditional_expr(Node *n);
    unsigned long resolve_binary_expr(Node *n);
    unsigned long resolve_cast_expr(Node *n);
    unsigned long resolve_unary_expr(Node *n);
    unsigned long resolve_prefix_expr(Node *n);
    unsigned long resolve_postfix_expr(Node *n);
    unsigned long resolve_subscript_expr(Node *n);
    unsigned long resolve_function_call(Node *n);

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
        case CAST_EXPR:
            return resolve_cast_expr(n);
        case PREFIX_EXPR:
            return resolve_prefix_expr(n);
        case POSTFIX_EXPR:
            return resolve_postfix_expr(n);
        case SUBSCRIPT_EXPR:
            return resolve_subscript_expr(n);
        case FUNCTION_CALL:
            return resolve_function_call(n);
        case IDENTIFIER_EXPR:
            set_symbol_table_entry(n, td->dummy_symbol);
            return VARIABLE_VALUE;
        case STRING_CONSTANT:
            return NON_INTEGRAL_VALUE;
        default:
            return UNSPECIFIED_VALUE;
    }
}

/* helpers for resolve constant expr */
Boolean invalid_operand(long operand) {
    switch (operand) {
        case NON_INTEGRAL_VALUE:
        case VARIABLE_VALUE:
        case UNSPECIFIED_VALUE:
            return TRUE;
        default:
            return FALSE;
    }
}

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

unsigned long resolve_cast_expr(Node *n) {
    unsigned long child2;
    child2 = resolve_constant_expr(n->children.child2);
    if (invalid_operand(child2)) {
        return child2;
    }
    return CAST_VALUE;
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

unsigned long resolve_prefix_expr(Node *n) {
    unsigned long child1;
    child1 = resolve_constant_expr(n->children.child1);
    if (invalid_operand(child1)) {
        return child1;
    }
    switch (n->data.attributes[OPERATOR]) {
        case INCREMENT:
            return ++child1;
        case DECREMENT:
            return --child1;
        default:
            return VARIABLE_VALUE;
    }
}

unsigned long resolve_postfix_expr(Node *n) {
    unsigned long child1;
    child1 = resolve_constant_expr(n->children.child1);
    if (invalid_operand(child1)) {
        return child1;
    }
    switch (n->data.attributes[OPERATOR]) {
        case INCREMENT:
            return child1++;
        case DECREMENT:
            return child1++;
        default:
            return VARIABLE_VALUE;
    }
}

unsigned long resolve_subscript_expr(Node *n) {
    unsigned long child1, child2;
    child1 = resolve_constant_expr(n->children.child1);
    child2 = resolve_constant_expr(n->children.child2);
    if (invalid_operand(child1)) {
        return child1;
    }
    if (invalid_operand(child2)) {
        return child2;
    }
    return VARIABLE_VALUE;
}

unsigned long resolve_function_call(Node *n) {
    unsigned long child1, child2;
    child1 = resolve_constant_expr(n->children.child1);
    child2 = resolve_constant_expr(n->children.child2);
    if (invalid_operand(child1)) {
        return child1;
    }
    if (invalid_operand(child2)) {
        return child2;
    }
    return VARIABLE_VALUE;
}

/*
 * create_symbol_if_necessary
 * Purpose:
 *      create the "current symbol" if it does not already exist
 * Parameters
 *      td: running traversal data
 * Returns:
 *      None
 * Side Effects:
 *      If it does create a new symbol:
 *          Allocates heap storage
 *          Sets the new symbol's type to the current base type
 *          Updates td to have a reference to the new symbol
 */
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

void record_current_symbol(TraversalData *td, Node *n) {
    /* have an identifier that should become a symbol table entry */
    Symbol *s = td->current_symbol;
    validate_symbol(s, td);
    /* add to appropriate symbol table and parse tree */
    /* cases:
     * any non-prototype declarators including function definitions
     * prototype function declarator: put in prototype symbol table
     * prototype function parameters: do NOT put in any symbol table
     */
    if (!td->function_prototype) {
        append_symbol(get_current_st(td->stc), s);
        set_symbol_table_entry(n, s);
    } else if (symbol_outer_type(s) == FUNCTION) {
        append_function_prototype(td->stc->function_prototypes, s);
        set_symbol_table_entry(n, s);
    } else {
        set_symbol_table_entry(n, td->dummy_symbol);
    }
}

void validate_symbol(Symbol *s, TraversalData *td) {
    /* only checks functions, are there any other cases? */
    if (symbol_outer_type(s) == FUNCTION) {
        validate_function_symbol(s, td);
    }
}

void validate_function_symbol(Symbol *s, TraversalData *td) {
    /* functions may be declared only at file scope */
    int scope = st_scope(get_current_st(td->stc));
    if (scope != TOP_LEVEL_SCOPE) {
        handle_symbol_error(STE_FUNC_DECL_SCOPE, get_symbol_name(s));
    }
    /* check that definition matches its prototype if present */
    if (td->function_def_spec) {
        Symbol *prototype;
        prototype = find_prototype(td->stc->function_prototypes,
                                    get_symbol_name(s));
        if (prototype != NULL && !symbols_same_type(s, prototype)) {
            handle_symbol_error(STE_PROTO_MISMATCH, get_symbol_name(s));
        }
    }
}

void validate_statement_labels(TraversalData *td) {
    Symbol *cur = st_symbols(td->stc->current_st[STATEMENT_LABELS]);
    while (cur != NULL) {
        if (!label_is_defined(cur)) {
            handle_symbol_error(STE_LAB_UNDEFINED, get_symbol_name(cur));
        }
        cur = cur->next;
    }
}

void print_symbol(FILE *out, Symbol *s) {
    if (s == td->dummy_symbol) {
        return;
    }
    fprintf(out, "\n/*\n");
    fprintf(out, " * symbol: %s\n", get_symbol_name(s));
    fprintf(out, " * type: %s\n", symbol_type_string(s));
    if (symbol_outer_type(s) == FUNCTION) {
        print_symbol_param_list(out, s);
    }
    fprintf(out, " *\n");
    print_symbol_table(out, get_symbol_table(s));
    fprintf(out, " */\n");
}

void print_symbol_param_list(FILE *out, Symbol *s) {
    FunctionParameter *fp = first_parameter(s);
    char *param_name;
    fprintf(out, " * parameters:\n");
    while (fp != NULL) {
        param_name = get_parameter_name(fp);
        param_name = strcmp(param_name, "") == 0 ? "(none)" : param_name;
        fprintf(out, " * type: %s", parameter_type_string(fp));
        fprintf(out, ", name: %s\n", param_name);
        fp = fp->next;
    }
}

void print_symbol_table(FILE *out, SymbolTable *st) {
    fprintf(out, " * symbol table:\n");
    fprintf(out, " * scope: %s\n", st_scope_name(st));
    fprintf(out, " * overloading class: %s\n", st_overloading_class_name(st));
}

