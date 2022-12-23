/**
 * @file   utility_tests.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  Test utility functions in sky framework
 * @date   2022-12-23
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef TESTS_UTILITY_TESTS_HPP
#define TESTS_UTILITY_TESTS_HPP

#include "gtest/gtest.h"
#include "utility.hpp"

TEST(sky_utility, crc) {
    char const text[] = "Hello, World!";
    // Skips null termination
    auto const& crc = sky::crc_8(reinterpret_cast<std::uint8_t const*>(text), sky::length_of(text) - 1);
    ASSERT_EQ(crc, 0x87);
}

#endif  // !TESTS_UTILITY_TESTS_HPP
