/**
 * @file    tests.cpp
 * @author  Pratchaya Khansomboon (me@mononerv.dev)
 * @brief   Test runner
 * @date    2022-11-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "gtest/gtest.h"

#include "sky.hpp"

TEST(sky_framework, make_buffer_from_mcp) {
    // TODO: Test make_mcp_buffer to output correct value
    ASSERT_TRUE(false);
}

TEST(sky_framework, make_mcp_from_buffer) {
    // TODO: Test make_mcp from buffer
    ASSERT_TRUE(false);
}

TEST(sky_framework, convert_address_to_u32) {
    // TODO: Test address_t conversion to u32
    ASSERT_TRUE(false);
}

TEST(sky_framework, convert_u32_to_address) {
    // TODO: Test u32 value conversion to address_t
    ASSERT_TRUE(false);
}

auto main(int argc, char const* argv[]) -> int {
    testing::InitGoogleTest(&argc, (char**)argv);
    return RUN_ALL_TESTS();
}
