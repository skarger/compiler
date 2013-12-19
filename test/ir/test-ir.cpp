#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "gtest/gtest.h"

extern "C" {
#include "../../src/include/ir.h"

#include "../../src/include/cmpl.h"
#include "../../src/include/lexer.h"
#include "../../y.tab.h"
#include "../../src/include/parse-tree.h"
#include "../../src/include/parser.h"
#include "../../src/include/symbol-utils.h"
#include "../../src/include/symbol-collection.h"
}

extern FILE *yyin;

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

class IrTest : public ::testing::Test {    
  protected:
    FILE *test_input;
    YYSTYPE data;
    IrList *ir_list;
    Node *id_expr;
    Node *num_const;
    Node *assign_expr;
    Node *root;
    /* 
     * Opens a pipe for directing input. Anything printed to 
     * test_input will be read by yyparse.
     */
    void SetUp() {
        int fds[2];
        ASSERT_EQ(0, pipe(fds));

        yyin = fdopen(fds[0], "r");
        ASSERT_TRUE(yyin);

        test_input = fdopen(fds[1], "w");
        ASSERT_TRUE(test_input);

        ir_list = create_ir_list(); /* redundant? */
        start_ir_computation();
    }
    
    void TearDown() {

    }

    void set_data_string(char str[]) {
        data = (YYSTYPE) create_string(strlen(str));
        strcpy( ((struct String *) data)->str, str );
    }

    void set_data_number(char str[]) {
        data = create_number(str);
    }

    void set_int_symbol(Node *n) {
        Symbol *s = create_symbol();
        push_symbol_type(s, SIGNED_INT);
        set_symbol_table_entry(n, s);
    }

    void create_id_expr(char str[]) {
        set_data_string(str);
        id_expr = create_node(IDENTIFIER_EXPR, data);
    }

    void create_num_constant(char str[]) {
        set_data_number(str);
        num_const = create_node(NUMBER_CONSTANT, data);
    }

    void compute_ir_constant_assignment() {
        char s[] = "a";
        create_id_expr(s);
        set_int_symbol(id_expr);
        char t[] = "1";
        create_num_constant(t);
        assign_expr = create_node(ASSIGNMENT_EXPR, ASSIGN, id_expr, num_const);
        compute_ir(assign_expr, ir_list);
    }

    void compute_ir_binary_expression() {
        char s[] = "a";
        create_id_expr(s);
        set_int_symbol(id_expr);
        char t[] = "1";
        create_num_constant(t);
        root = create_node(BINARY_EXPR, LOGICAL_OR, id_expr, num_const);
        compute_ir(root, ir_list);
    }

    void ExpectIRNode(void) {
        IrNode *ir_node = construct_ir_node(LOAD_ADDR);
        EXPECT_EQ(LOAD_ADDR, instruction(ir_node));
    }

    void ExpectIRList(void) {
        IrNode *ir_node1 = construct_ir_node(LOAD_ADDR);
        IrNode *ir_node2 = construct_ir_node(LOAD_WORD_INDIRECT);
        IrList *ir_list = create_ir_list();

        EXPECT_EQ(NULL, ir_list->head);
        EXPECT_EQ(NULL, ir_list->tail);
        EXPECT_EQ(NULL, ir_list->cur);

        append_ir_node(ir_node1, ir_list);

        EXPECT_EQ(ir_node1, ir_list->head);
        EXPECT_EQ(ir_node1, ir_list->tail);
        EXPECT_EQ(NULL, ir_list->head->next);
        EXPECT_EQ(NULL, ir_list->tail->next);
        EXPECT_EQ(NULL, ir_list->head->prev);
        EXPECT_EQ(NULL, ir_list->tail->prev);

        append_ir_node(ir_node2, ir_list);
        EXPECT_EQ(ir_node1, ir_list->head);
        EXPECT_EQ(ir_node2, ir_list->tail);
        EXPECT_EQ(ir_node2, ir_list->head->next);
        EXPECT_EQ(NULL, ir_list->tail->next);
        EXPECT_EQ(NULL, ir_list->head->prev);
        EXPECT_EQ(ir_node1, ir_list->tail->prev);
    }
};

TEST_F(IrTest, ValTest) {
  fputs("/", test_input);
  fclose(test_input);

  EXPECT_EQ(0, 0);
}

TEST_F(IrTest, IrNodeCreation) { this->ExpectIRNode(); }

TEST_F(IrTest, IrListCreation) { this->ExpectIRList(); }

TEST_F(IrTest, IrListSimpleAssignment) {
    this->compute_ir_constant_assignment();
    EXPECT_EQ(LOAD_ADDRESS, instruction(ir_list->head));
    EXPECT_EQ(LOAD_CONSTANT, instruction(ir_list->head->next));
    EXPECT_EQ(STORE_WORD_INDIRECT, instruction(ir_list->tail));
}

TEST_F(IrTest, IrListBinaryExpression) {
    this->compute_ir_binary_expression();
    EXPECT_EQ(LOAD_ADDRESS, instruction(ir_list->head));
    EXPECT_EQ(LOAD_CONSTANT, instruction(ir_list->head->next));
    EXPECT_EQ(LOG_OR, instruction(ir_list->tail));
}

TEST_F(IrTest, LValue) {
    this->compute_ir_constant_assignment();
    EXPECT_EQ(FALSE, node_is_lvalue(assign_expr));
    EXPECT_EQ(TRUE, node_is_lvalue(id_expr));
    EXPECT_EQ(FALSE, node_is_lvalue(num_const));
}

TEST_F(IrTest, Return) {
    Node *x = create_node(RETURN_STATEMENT, NULL);
    EXPECT_EQ(0, 0);
}



/*
Source:
int a;

int main(void) {
    a = 1;
}

Suppose the assignment expression "a = 1" has a parse tree like this:
        ASSIGNMENT_EXPR (op: ASSIGN)
        /               \
IDENTIFIER_EXPR     NUMBER_CONSTANT

Doing a post order traversal, compute the following info:

IDENTIFIER_EXPR
type: signed int
lvalue: yes
IR: loadaddress($r0, a)
location:  $r0

NUMBER_CONSTANT
type: signed int
lvalue: no
IR: loadsignedint($r1, 1)
location: $r1

ASSIGNMENT_EXPR (op: ASSIGN)
type: signed int
lvalue: no
IR: loadaddress($r0, a), loadsignedint($r1, 1), storewordindirect($r1, $r0)
location: $r0
*/
