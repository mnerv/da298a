/**
 * @file    tests.cpp
 * @author  Pratchaya Khansomboon (me@mononerv.dev)
 * @author  Petter Rignell
 * @author  Christian Heisterkamp
 * @brief   Test runner
 * @date    2022-11-18
 * 
 * @copyright Copyright (c) 2022
 */
#include "gtest/gtest.h"
#include "fmt/format.h"
#include "sky.hpp"

TEST(sky_framework, make_buffer_from_mcp) {
    // TODO: Test make_mcp_buffer to output correct value
    ASSERT_TRUE(false);
}

TEST(sky_framework, make_mcp_from_buffer) {
    //In sky.hpp -> auto make_mcp(mcp_buffer const& src) -> mcp;
    sky::mcp_buffer src = { 0x00, 0x04, 0x00 };
    sky::address_t address_source = { 0x00, 0x00, 0x01 };
    sky::mcp expected_mcp = {
        1,
        {0x00, 0x04, 0x00},
        {0x00, 0x01, 0x00},
        {0x00},
        1
    };

    
    mcp message{
        1,
        {0, 0, 255},
        {0, 0, 2},
        {15},
        1
    };

    uint8_t* data = (uint8_t*)&message;
    for (auto i = 0; i < sizeof(mcp); ++i) {
        fmt::print("{:#04x} ", data[i]);
    }
    fmt::print("\n");

    //sky::mcp the_mcp = 
    //EXPECT_EQ(expected_value, output);
    fmt::print("{}\n", the_mcp);

}

TEST(sky_framework, convert_address_to_u32) {
    constexpr sky::address_t address = { 0x00, 0x04, 0x00 };
    uint32_t output = sky::address_to_u32(address);
    uint32_t expected_value = 1024;
    fmt::print("{}\n", output);

    EXPECT_EQ(expected_value, output);
}

TEST(sky_framework, convert_u32_to_address) {
    constexpr uint32_t test_address = 1024;
    sky::address_t output;
    sky::u32_to_address(output, test_address);
    constexpr sky::address_t expected_value = { 0x00, 0x04, 0x00 };
    for (size_t i = 0; i < sizeof(sky::address_t); ++i) {
        EXPECT_EQ(expected_value[i], output[i]);
        fmt::print("{:#04x} ", output[i]);
    }
    fmt::print("\n");
   
}

auto main(int argc, char const* argv[]) -> int {
    testing::InitGoogleTest(&argc, (char**)argv);
    return RUN_ALL_TESTS();
}
