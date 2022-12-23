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

#include "mcp_tests.hpp"
#include "topo_tests.hpp"
#include "utility_tests.hpp"

auto main(int argc, char const* argv[]) -> int {
    testing::InitGoogleTest(&argc, (char**)argv);
    return RUN_ALL_TESTS();
}
