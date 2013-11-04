#include <stdio.h>
#include <stdlib.h>
#include "../include/symbol.h"
#include "../../y.tab.h"

char *get_test_result_name(int res);
char *get_overloading_class_name(int oc);

void test_transition(Node *n, enum node_type nt, int action,
        enum scope_state expected_state, int expected_scope, int expected_oc) {
    int test_state, test_scope, test_oc;

    n->n_type = nt;
    transition_scope(n, action);
    enum scope_state cur = get_state();
    int scope = get_scope();
    int oc = get_overloading_class();
    printf("current state: %19s scope: %d overloading class: %16s | ",
            get_scope_state_name(cur), get_scope(), get_overloading_class_name(oc));

    test_state = (cur == expected_state ? PASS : FAIL);
    test_scope = (scope == expected_scope ? PASS : FAIL);
    test_oc    = (oc == expected_oc ? PASS : FAIL);
    printf("state: %s ", get_test_result_name(test_state));
    printf("scope: %s ", get_test_result_name(test_scope));
    printf("overld. class: %s ", get_test_result_name(test_oc));
    printf("\n");
}

char *get_test_result_name(int res) {
    if (res == PASS) return "PASS";
    if (res == FAIL) return "FAIL";
}

int main() {
    initialize_fsm();
    enum scope_state cur = get_state();
    int oc = get_overloading_class();
    printf("initial state: %19s scope: %d overloading class: %16s\n",
            get_scope_state_name(cur), get_scope(), get_overloading_class_name(oc));

    Node *n = malloc(sizeof(Node));

    printf("top level decl:\n");
    test_transition(n, DECL, START, TOP_LEVEL, 0, OTHER_NAMES);
    test_transition(n, DECL, END, TOP_LEVEL, 0, OTHER_NAMES);

    printf("function definition:\n");
    test_transition(n, FUNCTION_DEFINITION, START, FUNCTION_DEF, 0, OTHER_NAMES);
    test_transition(n, PARAMETER_LIST, START, FUNCTION_DEF_PARAMETERS, 1, OTHER_NAMES);
    test_transition(n, COMPOUND_STATEMENT, START, FUNCTION_BODY, 1, OTHER_NAMES);
    test_transition(n, COMPOUND_STATEMENT, START, BLOCK, 2, OTHER_NAMES);
    test_transition(n, COMPOUND_STATEMENT, START, BLOCK, 3, OTHER_NAMES);

    test_transition(n, GOTO_STATEMENT, START, BLOCK, 3, STATEMENT_LABELS);
    test_transition(n, IDENTIFIER, END, BLOCK, 3, OTHER_NAMES);

    test_transition(n, COMPOUND_STATEMENT, END, BLOCK, 2, OTHER_NAMES);
    test_transition(n, COMPOUND_STATEMENT, END, FUNCTION_BODY, 1, OTHER_NAMES);

    test_transition(n, LABELED_STATEMENT, START, FUNCTION_BODY, 1, STATEMENT_LABELS);
    test_transition(n, IDENTIFIER, END, FUNCTION_BODY, 1, OTHER_NAMES);

    test_transition(n, COMPOUND_STATEMENT, END, TOP_LEVEL, 0, OTHER_NAMES);


    printf("function prototype:\n");
    initialize_fsm();
    n->n_type = TOP_LEVEL;
    test_transition(n, FUNCTION_DECLARATOR, START, FUNCTION_PROTOTYPE, 0, OTHER_NAMES);
    test_transition(n, PARAMETER_DECL, START, FUNCTION_PROTO_PARAMETERS, 1, OTHER_NAMES);
    test_transition(n, FUNCTION_DECLARATOR, END, TOP_LEVEL, 0, OTHER_NAMES);

    printf("function prototype:\n");
    test_transition(n, FUNCTION_DECLARATOR, START, FUNCTION_PROTOTYPE, 0, OTHER_NAMES);
    /* set the type to void to make the node match a possible function param */
    n->data.symbols[TYPE_SPEC] = VOID;
    test_transition(n, TYPE_SPECIFIER, START, FUNCTION_PROTO_PARAMETERS, 1, OTHER_NAMES);
    test_transition(n, FUNCTION_DECLARATOR, END, TOP_LEVEL, 0, OTHER_NAMES);

    return 0;
}
