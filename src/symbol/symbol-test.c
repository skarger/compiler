#include <stdio.h>
#include "../include/symbol.h"

char *get_test_result_name(int res);
void test_transition(Node *n, enum node_type nt, int action,
                enum scope_state expected_state, int expected_scope);

void test_transition(Node *n, enum node_type nt, int action,
                    enum scope_state expected_state, int expected_scope) {
    int test_state, test_scope;

    n->n_type = nt;
    transition_scope(n, action);
    enum scope_state cur = get_state();
    int scope = get_scope();
    printf("current state: %s scope: %d   ", get_scope_state_name(cur), get_scope());

    test_state = (cur == expected_state ? PASS : FAIL);
    test_scope = (scope == expected_scope ? PASS : FAIL);
    printf("scope state: %s ", get_test_result_name(test_state));
    printf("scope: %s ", get_test_result_name(test_scope));
    printf("\n");
}

char *get_test_result_name(int res) {
    if (res == PASS) return "PASS";
    if (res == FAIL) return "FAIL";
}

