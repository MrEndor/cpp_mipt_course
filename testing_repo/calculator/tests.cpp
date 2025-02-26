/** @author yaishenka
    @date 04.01.2023 */
#include <gtest/gtest.h>
#include "Calculator.hpp"

TEST(SimpleInt, Plus) {
  std::string expr = "1+2";
  int answer = 3;

  EXPECT_EQ(Calculator<decltype(answer)>::CalculateExpr(expr), answer);
}

TEST(SimpleInt, Minus) {
  std::string expr = "1-2";
  int answer = -1;

  EXPECT_EQ(Calculator<decltype(answer)>::CalculateExpr(expr), answer);
}

TEST(SimpleInt, Product) {
  std::string expr = "2 * 5";
  int answer = 10;

  EXPECT_EQ(Calculator<decltype(answer)>::CalculateExpr(expr), answer);
}

TEST(SimpleInt, Devision) {
  std::string expr = "5 / 2";
  int answer = 2;

  EXPECT_EQ(Calculator<decltype(answer)>::CalculateExpr(expr), answer);
}

TEST(SimpleInt, UnaryMinus) {
  std::string expr = "1 + -2";
  int answer = -1;

  EXPECT_EQ(Calculator<decltype(answer)>::CalculateExpr(expr), answer);
}

TEST(SimpleInt, UnaryPlus) {
  std::string expr = "1 + +2";
  int answer = 3;

  EXPECT_EQ(Calculator<decltype(answer)>::CalculateExpr(expr), answer);
}

TEST(SimpleInt, Brackets) {
  std::string expr = "5 * (10 - 2)";
  int answer = 40;

  EXPECT_EQ(Calculator<decltype(answer)>::CalculateExpr(expr), answer);
}

TEST(SimpleDouble, Plus) {
  std::string expr = "1.2+2.2";
  double answer = 3.4;

  EXPECT_FLOAT_EQ(Calculator<decltype(answer)>::CalculateExpr(expr), answer);
}

TEST(SimpleDouble, Minus) {
  std::string expr = "3 - 1.2";
  double answer = 1.8;

  EXPECT_FLOAT_EQ(Calculator<decltype(answer)>::CalculateExpr(expr), answer);
}

TEST(SimpleDouble, Product) {
  std::string expr = "3 * 1.2";
  double answer = 3.6;

  EXPECT_FLOAT_EQ(Calculator<decltype(answer)>::CalculateExpr(expr), answer);
}

TEST(Simpledouble, Devision) {
  std::string expr = "5 / 2";
  double answer = 2.5;

  EXPECT_FLOAT_EQ(Calculator<decltype(answer)>::CalculateExpr(expr), answer);
}

TEST(Simpledouble, UnaryMinus) {
  std::string expr = "1.5 + -2";
  double answer = -0.5;

  EXPECT_FLOAT_EQ(Calculator<decltype(answer)>::CalculateExpr(expr), answer);
}

TEST(Simpledouble, UnaryPlus) {
  std::string expr = "1.5 + +2";
  double answer = 3.5;

  EXPECT_FLOAT_EQ(Calculator<decltype(answer)>::CalculateExpr(expr), answer);
}

TEST(Simpledouble, Brackets) {
  std::string expr = "5.1 * (10.33 - 2.1)";
  double answer = 41.973;

  EXPECT_FLOAT_EQ(Calculator<decltype(answer)>::CalculateExpr(expr), answer);
}

TEST(HardInt, Hard1) {
  std::string expr = "((1 + (2 + (3 + (4 + (5))))))";
  int answer = 15;

  EXPECT_EQ(Calculator<decltype(answer)>::CalculateExpr(expr), answer);
}

TEST(HardInt, Hard2) {
  std::string expr = "( 5 + 3 ) * ( -5 - -7 )";
  int answer = 16;

  EXPECT_EQ(Calculator<decltype(answer)>::CalculateExpr(expr), answer);
}

TEST(ExceptionsInt, Exeption1) {
  std::string expr = "((((0)";

  ASSERT_THROW(Calculator<int>::CalculateExpr(expr), std::exception);
}

TEST(ExceptionsInt, Exeption2) {
  std::string expr = "( 10 - 5 ) ( 5 * 10 )";

  ASSERT_THROW(Calculator<int>::CalculateExpr(expr), std::exception);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
