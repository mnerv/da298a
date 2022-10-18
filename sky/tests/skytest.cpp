/**
 * @file   skytest.cpp
 * @author Pratchaya Khansomboon <me@mononerv.dev>
 * @brief  sky framework test suites
 * @date   2022-10-24
 *
 * @copyright Copyright (c) 2022
 */
#include <string>

#include "gtest/gtest.h"

TEST(sky, test) {
    ASSERT_TRUE(false);
}

auto main(int argc, char const* argv[]) -> int {
    testing::InitGoogleTest(&argc, (char**)argv);
    return RUN_ALL_TESTS();
}
