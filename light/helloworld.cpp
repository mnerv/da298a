/**
 * @file   helloworld.cpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  light program is simulation for sky framework.
 * @date   2022-10-18
 *
 * @copyright Copyright (c) 2022
 */
#include "fmt/format.h"
#include "sky.hpp"

auto main([[maybe_unused]]int argc, [[maybe_unused]]char const* argv[]) -> int {
    fmt::print("Hello, {}!", sky::hello());
    return 0;
}

