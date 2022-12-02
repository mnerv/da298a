/**
 * @file   sky.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @author Christian Heisterkamp
 * @author Petter Rignell
 * @brief  sky framework
 * @date   2022-10-19
 *
 * @copyright Copyright (c) 2022
 */
#ifndef SKY_SKY_HPP
#define SKY_SKY_HPP

#include <cstddef>

#include "mcp.hpp"
#include "topo.hpp"
#include "queue.hpp"

namespace sky {
template <typename T, std::size_t N>
constexpr auto length_of(T (&)[N]) -> std::size_t {
    return N;
}
}

#endif  // !SKY_SKY_HPP
