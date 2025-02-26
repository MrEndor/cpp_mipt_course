#include "sum_of_two.hpp"
#include <gtest/gtest.h>

TEST(CORRECTNESS, SMALL) {
  EXPECT_EQ(SumOfTwo(2, 3), 5);
}

TEST(CORRECTNESS, Medium) {
  EXPECT_EQ(SumOfTwo(228, 1337), 1565);
}

TEST(CORRECTNESS, Large) {
  EXPECT_EQ(SumOfTwo(1 << 30, (1 << 30) - 1), (1 << 30) - 1 + (1 << 30));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
