/**
 * @file   mcp.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @author Christian Heisterkamp
 * @brief  Message Control Protocol structure and helper functions.
 * @date   2022-11-23
 *
 * @copyright Copyright (c) 2022
 */
#ifndef SKY_MCP_HPP
#define SKY_MCP_HPP
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

auto mcp_make_buffer(mcp_buffer& dest, mcp const& src) -> void;
auto mcp_make_from_buffer(mcp_buffer const& src) -> mcp;
auto mcp_address_to_u32(address_t const& addr) -> uint32_t;
auto mcp_u32_to_address(address_t& dest, uint32_t const& addr) -> void;
}

#endif  // !SKY_MCP_HPP
