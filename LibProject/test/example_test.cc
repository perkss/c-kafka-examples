#include "gtest/gtest.h"

TEST(FirstUnitTest, PositiveNos) {
    EXPECT_EQ (18.2, 18.2);
    EXPECT_EQ (25.4, 25.4);
    EXPECT_EQ (50, 50);
}

TEST (FirstUnitTest, ZeroAndNegativeNos) {
    ASSERT_EQ (0.0, 0.0);
    ASSERT_EQ (-3, -3);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}
