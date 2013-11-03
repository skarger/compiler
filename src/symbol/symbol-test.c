#include <stdio.h>
#include "../include/symbol.h"

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
    printf("current state: %s scope: %d overloading class: %s ",
            get_scope_state_name(cur), get_scope(), get_overloading_class_name(oc));

    test_state = (cur == expected_state ? PASS : FAIL);
    test_scope = (scope == expected_scope ? PASS : FAIL);
    test_oc    = (oc == expected_oc ? PASS : FAIL);
    printf("scope state: %s ", get_test_result_name(test_state));
    printf("scope: %s ", get_test_result_name(test_scope));
    printf("overloading class: %s ", get_test_result_name(test_oc));
    printf("\n");
}

char *get_test_result_name(int res) {
    if (res == PASS) return "PASS";
    if (res == FAIL) return "FAIL";
}
