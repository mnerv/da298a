/**
 * @file   sky.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @author Christian Heisterkamp
 * @brief  sky framework
 * @date   2022-10-19
 *
 * @copyright Copyright (c) 2022
 */
#ifndef SKY_SKY_HPP
#define SKY_SKY_HPP
#include <stdint.h>

namespace sky {
using address_t = uint8_t[3];
using payload_t = uint8_t[15];

//message control protocol
struct mcp {
    uint8_t   type;
    address_t source;
    address_t destination;
    payload_t payload;
    uint8_t   crc;
};

constexpr auto mcp_size = sizeof(mcp);
using mcp_buffer = uint8_t[mcp_size];

auto make_mcp_buffer(mcp_buffer& dest, mcp const& src) -> void;
auto make_mcp(mcp_buffer const& src) -> mcp;

auto hello() -> char const*;
}

#endif  // SKY_SKY_HPP
