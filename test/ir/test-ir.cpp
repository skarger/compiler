#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "gtest/gtest.h"

extern "C" {
#include "../../src/include/ir.h"

#include "../../src/include/traverse.h"
#include "../../src/include/lexer.h"
#include "../../y.tab.h"
#include "../../src/include/parse-tree.h"
#include "../../src/include/parser.h"
#include "../../src/include/symbol-utils.h"
}

extern FILE *yyin;

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

class IrTest : public ::testing::Test {    
  protected:
    FILE *test_input;
    
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
    }
    
    void TearDown() {

    }
    
    void ExpectReg(void) {
        char *r0 = current_reg();
        char *r1 = next_reg();
        char *r2 = current_reg();
        EXPECT_STREQ("$r0", r0);
        EXPECT_STREQ("$r1", r1);
        EXPECT_EQ(0, strcmp(r1, r2));
    }

    void ExpectIR(void) {
        YYSTYPE data;
        char str1[] = "a";
        data = (YYSTYPE) create_string(1);
        strcpy( ((struct String *) data)->str, str1 );
        Node *var = create_node(IDENTIFIER, data);
        set_node_type(var, IDENTIFIER_EXPR);

        char str2[] = "1";
        data = create_number(str2);
        Node *val = create_node(NUMBER_CONSTANT, data);

        Node *bin_expr = create_node(BINARY_EXPR, ASSIGN, var, val);

        Symbol *s = create_symbol();
        push_symbol_type(s, SIGNED_INT);
        set_symbol_table_entry(var, s);

        IrList *ir_list = create_ir_list();


        compute_ir(bin_expr, ir_list);


        EXPECT_EQ(0,0);
    }

    void ExpectIRList(void) {
        IrNode *ir_node1 = create_ir_node(LOAD_ADDR);
        IrNode *ir_node2 = create_ir_node(LOAD_INDIRECT_WORD);
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

TEST_F(IrTest, RegTest) { this->ExpectReg(); }

TEST_F(IrTest, IrList1) { this->ExpectIR(); }

TEST_F(IrTest, IrList2) { this->ExpectIRList(); }

/*
Source:
int a;

int main(void) {
    a = 1;
}

Suppose the assignment expression "a = 1" has a parse tree like this:
        BINARY_EXPR (op: ASSIGN)
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

BINARY_EXPR (op: ASSIGN)
type: signed int
lvalue: no
IR: loadaddress($r0, a), loadsignedint($r1, 1), storewordindirect($r1, $r0)
location: $r0
*/
