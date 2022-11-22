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
    constexpr uint32_t test_address = 1024;
    sky::address_t output;
    sky::u32_to_address(output, test_address);
    constexpr sky::address_t expected_value = { 0x00, 0x04, 0x00 };
    for (size_t i = 0; i < sizeof(sky::address_t); ++i) {
        EXPECT_EQ(expected_value[i], output[i]);
    }
}

auto main(int argc, char const* argv[]) -> int {
    testing::InitGoogleTest(&argc, (char**)argv);
    return RUN_ALL_TESTS();
}
