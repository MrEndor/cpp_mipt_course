#include "ring_buffer.hpp"
#include <gtest/gtest.h>

#include <random>


TEST(Correctness, Empty) {
  RingBuffer buffer(1);

  EXPECT_TRUE(0u == buffer.Size());
  EXPECT_TRUE(true == buffer.Empty());
}

TEST(Correctness, PushAndPop) {
  RingBuffer buffer(2);

  int i;
  EXPECT_TRUE(buffer.TryPush(0));
  EXPECT_TRUE(buffer.TryPush(1));
  EXPECT_TRUE(!buffer.TryPush(2));

  EXPECT_TRUE(2u == buffer.Size());
  EXPECT_TRUE(!buffer.Empty());

  EXPECT_TRUE(buffer.TryPop(&i));
  EXPECT_TRUE(0 == i);
  EXPECT_TRUE(buffer.TryPop(&i));
  EXPECT_TRUE(1 == i);

  EXPECT_TRUE(!buffer.TryPop(&i));
  EXPECT_TRUE(0u == buffer.Size());
  EXPECT_TRUE(buffer.Empty());
}

TEST(Stress, Random) {
  std::mt19937 gen(311);
  std::uniform_int_distribution<> distrib_comm(0, 1);
  RingBuffer buffer(10);

  int next_element = 0;
  int next_received_element = 0;
  for (int i = 0; i < 100000; ++i) {
    if (distrib_comm(gen) == 0) {
      if (buffer.TryPush(next_element)) {
        next_element++;
      }
    } else {
      int element;
      if (buffer.TryPop(&element)) {
        EXPECT_EQ(next_received_element, element);
        next_received_element++;
      }
    }
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
