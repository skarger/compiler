#include <stdio.h>
#include <unistd.h>

#include "gtest/gtest.h"

#include "../../src/include/ir.h"

extern FILE *yyin;
extern "C" int f(int a);

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

class IrTest : public ::testing::Test {    
  protected:
    FILE *test_input;
    
    /* 
     * Opens a pipe for directing input. Anything printed to 
     * test_input will be read by lex.
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
    
    void ExpectIntString(int i, const char *input) {
      fputs(input, test_input);
      fclose(test_input);
      
      EXPECT_EQ(0, f(0));
    }
};

TEST_F(IrTest, ValTest) {
  fputs("/", test_input);
  fclose(test_input);

  EXPECT_EQ(0, f(0));
}

/* 
 * The next two tests are similar to SlashTest but have been refactored to use
 * the ExpectTokenOnly method from the fixture. */
TEST_F(IrTest, PlusTest) { this->ExpectIntString(1, "+"); }
