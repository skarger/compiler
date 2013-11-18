#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/include/symbol.h"
#include "../../src/include/literal.h"
#include "../../y.tab.h"

char *get_test_result_name(int res);
char *get_overloading_class_name(int oc);
void test_transition(Node *n, enum node_type nt, int action,
        enum scope_state expected_state, int expected_scope, int expected_oc);
void test_st_fsm();
void test_st_data();
void test_symbol_data();

int main() {
    test_st_fsm();
    printf("\n");
    test_st_data();
    printf("\n");
    test_symbol_data();
    return 0;
}

char *get_test_result_name(int res) {
    if (res == PASS) return "PASS";
    if (res == FAIL) return "FAIL";
}

int assert_equal_int(int i, int j) {
    return (i == j ? PASS : FAIL);
}

int assert_equal_string(char *str1, char *str2) {
    return (strcmp(str1, str2) == 0 ? PASS : FAIL);
}

int assert_true(enum Boolean b) {
    return (b == TRUE ? PASS : FAIL);
}

int assert_not_true(enum Boolean b) {
    return (b == FALSE ? PASS : FAIL);
}

void test_symbol_data() {
    int test_res;

    printf("*** Testing Symbol data methods ***\n");

    TypeNode *tn = create_type_node(SIGNED_CHAR);
    test_res = assert_equal_int(sizeof(*tn), sizeof(TypeNode));
    printf("%s TypeNode: size\n", get_test_result_name(test_res));

    test_res = assert_equal_int(SIGNED_CHAR, tn->type);
    printf("%s TypeNode: initial type\n", get_test_result_name(test_res));

    /* push_type */
    tn = push_type(tn, UNSIGNED_INT);
    test_res = assert_equal_int(UNSIGNED_INT, tn->type);
    printf("%s push_type: new type\n", get_test_result_name(test_res));

    TypeNode *tn2 = tn->next;
    test_res = assert_equal_int(SIGNED_CHAR, tn2->type);
    printf("%s push_type: original type\n", get_test_result_name(test_res));

    tn = push_type(tn, ARRAY);
    test_res = assert_equal_int(ARRAY, tn->type);
    printf("%s push_type: array type\n", get_test_result_name(test_res));

    set_array_size(tn, 5);
    test_res = assert_equal_int(5, get_array_size(tn));
    printf("%s set_array_size: value \n", get_test_result_name(test_res));

    tn = push_type(tn, FUNCTION);
    test_res = assert_equal_int(FUNCTION, tn->type);
    printf("%s push_type: function type\n", get_test_result_name(test_res));
    /* end push type */

    FunctionParameter *fp = create_function_parameter();
    test_res = assert_equal_int(sizeof(*fp), sizeof(FunctionParameter));
    printf("%s FunctionParameter: size\n", get_test_result_name(test_res));

    test_res = assert_equal_string("", fp->name);
    printf("%s FunctionParameter: initial name\n", get_test_result_name(test_res));

    set_function_parameter_name(fp, "x");
    test_res = assert_equal_string("x", fp->name);
    printf("%s FunctionParameter: name\n", get_test_result_name(test_res));

    push_parameter_type(fp, SIGNED_INT);
    test_res = assert_equal_int(SIGNED_INT, fp->type_tree->type);
    printf("%s push_parameter_type \n", get_test_result_name(test_res));

    FunctionParameter *fp1, *fp2;
    fp1 = fp;
    fp2 = create_function_parameter();
    push_parameter_type(fp2, SIGNED_INT);
    test_res = assert_true(parameters_same_type(fp1, fp2));
    printf("%s parameters_same_type \n", get_test_result_name(test_res));

    Symbol *s = create_symbol();
    test_res = assert_equal_int(sizeof(*s), sizeof(Symbol));
    printf("%s Symbol: size\n", get_test_result_name(test_res));

    push_symbol_type(s, SIGNED_SHORT);
    test_res = assert_equal_int(SIGNED_SHORT, s->type_tree->type);
    printf("%s push_symbol_type: 1\n", get_test_result_name(test_res));

    push_symbol_type(s, UNSIGNED_LONG);
    test_res = assert_equal_int(UNSIGNED_LONG, s->type_tree->type);
    printf("%s push_symbol_type: 2\n", get_test_result_name(test_res));

    Symbol *s1, *s2;
    s1 = s;
    s2 = create_symbol();
    push_symbol_type(s2, SIGNED_SHORT);
    push_symbol_type(s2, UNSIGNED_LONG);
    test_res = assert_true(symbols_same_type(s1, s2));
    printf("%s symbols_same_type: equal\n", get_test_result_name(test_res));

    /* different number of type pieces */
    push_symbol_type(s2, UNSIGNED_CHAR);
    test_res = assert_not_true(symbols_same_type(s1, s2));
    printf("%s symbols_same_type: not equal\n", get_test_result_name(test_res));

    /* bring types back to equality */
    push_symbol_type(s1, UNSIGNED_CHAR);
    test_res = assert_true(symbols_same_type(s1, s2));
    printf("%s symbols_same_type: equal\n", get_test_result_name(test_res));

    /* same number of type pieces but different type values */
    push_symbol_type(s1, SIGNED_INT);
    push_symbol_type(s2, UNSIGNED_INT);
    test_res = assert_not_true(symbols_same_type(s1, s2));
    printf("%s symbols_same_type: not equal\n", get_test_result_name(test_res));



}



void test_st_data() {
    int test_res;

    printf("*** Testing Symbol Table data methods ***\n");

    /* symbol table values depend on scope state */
    initialize_fsm();

    SymbolTableContainer *stc = create_st_container();

    /* verify expected size and values */
    test_res = (sizeof(*stc) == sizeof(SymbolTableContainer) ? PASS : FAIL);
    printf("%s SymbolTableContainer: size\n", get_test_result_name(test_res));

    test_res = (stc->symbol_tables[OTHER_NAMES] ==  NULL ? PASS : FAIL);
    printf("%s SymbolTableContainer: initialize other names\n", get_test_result_name(test_res));

    test_res = (stc->symbol_tables[STATEMENT_LABELS] ==  NULL ? PASS : FAIL);
    printf("%s SymbolTableContainer: initialize statement labels\n", get_test_result_name(test_res));

    test_res = (stc->current_st == NULL ? PASS : FAIL);
    printf("%s SymbolTableContainer: initialize current_st\n", get_test_result_name(test_res));

    test_res = (should_create_new_st() == TRUE ? PASS : FAIL);
    printf("%s should_create_new_st: initially true\n", get_test_result_name(test_res));

    SymbolTable *st = create_symbol_table();
    test_res = (should_create_new_st() == FALSE ? PASS : FAIL);
    printf("%s should_create_new_st: false after creating symbol table\n", get_test_result_name(test_res));

    test_res = (sizeof(*st) == sizeof(SymbolTable) ? PASS : FAIL);
    printf("%s SymbolTable: initialize size\n", get_test_result_name(test_res));

    test_res = (st->scope == TOP_LEVEL_SCOPE ? PASS : FAIL);
    printf("%s SymbolTable: initialize scope\n", get_test_result_name(test_res));

    test_res = (st->oc == OTHER_NAMES ? PASS : FAIL);
    printf("%s SymbolTable: initialize overloading class\n", get_test_result_name(test_res));

    /* check insertion */
    insert_symbol_table(st, stc);

    test_res = (stc->symbol_tables[OTHER_NAMES] == st ? PASS : FAIL);
    printf("%s insert_symbol_table: insert into container\n", get_test_result_name(test_res));

    test_res = (stc->current_st == st ? PASS : FAIL);
    printf("%s insert_symbol_table: update current_st\n", get_test_result_name(test_res));
}

void test_st_fsm() {
    printf("*** Testing Symbol Table Finite State Machine methods ***\n");

    initialize_fsm();
    enum scope_state cur = get_state();
    int oc = get_overloading_class();
    printf("initial state: %25s scope: %d overloading class: %16s\n",
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


    printf("function definition 2:\n");
    initialize_fsm();
    n->n_type = TOP_LEVEL;
    test_transition(n, FUNCTION_DEFINITION, START, FUNCTION_DEF, 0, OTHER_NAMES);
    test_transition(n, PARAMETER_DECL, START, FUNCTION_DEF_PARAMETERS, 1, OTHER_NAMES);
    test_transition(n, COMPOUND_STATEMENT, START, FUNCTION_BODY, 1, OTHER_NAMES);
    test_transition(n, COMPOUND_STATEMENT, END, TOP_LEVEL, 0, OTHER_NAMES);

    printf("function definition 3:\n");
    initialize_fsm();
    n->n_type = TOP_LEVEL;
    test_transition(n, FUNCTION_DEFINITION, START, FUNCTION_DEF, 0, OTHER_NAMES);
    /* set the type to void to make the node match a possible function param */
    n->data.attributes[TYPE_SPEC] = VOID;
    test_transition(n, TYPE_SPECIFIER, START, FUNCTION_DEF_PARAMETERS, 1, OTHER_NAMES);
    test_transition(n, COMPOUND_STATEMENT, START, FUNCTION_BODY, 1, OTHER_NAMES);
    test_transition(n, COMPOUND_STATEMENT, END, TOP_LEVEL, 0, OTHER_NAMES);
}

void test_transition(Node *n, enum node_type nt, int action,
        enum scope_state expected_state, int expected_scope, int expected_oc) {
    int test_state, test_scope, test_oc;

    n->n_type = nt;
    transition_scope(n, action);
    enum scope_state cur = get_state();
    int scope = get_scope();
    int oc = get_overloading_class();
    printf("current state: %25s scope: %d overloading class: %16s | ",
            get_scope_state_name(cur), get_scope(), get_overloading_class_name(oc));

    test_state = (cur == expected_state ? PASS : FAIL);
    test_scope = (scope == expected_scope ? PASS : FAIL);
    test_oc    = (oc == expected_oc ? PASS : FAIL);
    printf("state: %s ", get_test_result_name(test_state));
    printf("scope: %s ", get_test_result_name(test_scope));
    printf("overld. class: %s ", get_test_result_name(test_oc));
    printf("\n");
}
