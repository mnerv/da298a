/**TESTS_MCP_TESTS_HPP
 * @file   mcp_tests.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @author Christian Heisterkamp
 * @brief  Message Control Protocol structure and helper functions.
 * @date   2022-11-23
 *
 * @copyright Copyright (c) 2022
 */
#ifndef TESTS_MCP_TESTS_HPP
#define TESTS_MCP_TESTS_HPP
#include "mcp.hpp"
#include "gtest/gtest.h"

TEST(sky_framework, make_buffer_from_mcp) {
    // TODO: Test make_mcp_buffer to output correct value
    ASSERT_TRUE(false);
}

TEST(sky_framework, make_mcp_from_buffer) {
    //In sky.hpp -> auto make_mcp(mcp_buffer const& src) -> mcp;
    sky::mcp_buffer_t src = {};
    src[0] = 1;
    src[1] = 0x00;
    src[2] = 0x04;
    src[3] = 0x00;
    src[4] = 0x00;
    src[5] = 0x00;
    src[6] = 0x01;
    src[22] = 1;

    for (uint8_t i = 0; i < 15; i++) {
        src[i + 7] = 0x00;
    }
    sky::mcp expected_mcp = {
        1,
        {0x00, 0x04, 0x00},
        {0x00, 0x00, 0x01}, 
        {0x00},
        1
    };

    sky::mcp data = sky::mcp_make_from_buffer(src);
    EXPECT_EQ(expected_mcp.type, data.type);

    for (auto i = 0; i < 3; i++) {
        EXPECT_EQ(expected_mcp.source[i], data.source[i]);
        EXPECT_EQ(expected_mcp.destination[i], data.destination[i]);
    }

    for (auto i = 0; i < 15; i++) {
        EXPECT_EQ(expected_mcp.payload[i], data.payload[i]);
    }
    EXPECT_EQ(expected_mcp.crc, data.crc);

}

TEST(sky_mcp, convert_address_to_u32) {
    constexpr sky::address_t address = { 0x00, 0x04, 0x00 };
    uint32_t output = sky::mcp_address_to_u32(address);
    uint32_t expected_value = 1024;
    fmt::print("{}\n", output);

    EXPECT_EQ(expected_value, output);
}

TEST(sky_mcp, convert_u32_to_address) {
    constexpr uint32_t test_address = 1024;
    sky::address_t output;
    sky::mcp_u32_to_address(output, test_address);
    constexpr sky::address_t expected_value = { 0x00, 0x04, 0x00 };
    for (size_t i = 0; i < sizeof(sky::address_t); ++i) {
        EXPECT_EQ(expected_value[i], output[i]);
        fmt::print("{:#04x} ", output[i]);
    }
    fmt::print("\n");
   
}

#endif  // !TESTS_MCP_TESTS_HPP
