#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "gtest/gtest.h"

extern "C" {
#include "../../src/include/ir.h"
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
};

TEST_F(IrTest, ValTest) {
  fputs("/", test_input);
  fclose(test_input);

  EXPECT_EQ(0, 0);
}

TEST_F(IrTest, RegTest) { this->ExpectReg(); }
