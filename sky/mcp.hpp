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
#include <cstdint>
#include <cstddef>

namespace sky {
constexpr std::size_t address_size = 3;
constexpr std::size_t payload_size = 15;
using address_t = uint8_t[address_size];
using payload_t = uint8_t[payload_size];

//message control protocol
struct mcp {
    uint8_t   type;
    address_t source;
    address_t destination;
    payload_t payload;
    uint8_t   crc;
};

constexpr std::size_t mcp_buffer_size = sizeof(mcp);
using mcp_buffer_t = uint8_t[mcp_buffer_size];

auto mcp_make_buffer(mcp_buffer_t& dest, mcp const& src) -> void;
auto mcp_make_from_buffer(mcp_buffer_t const& src) -> mcp;
auto mcp_address_to_u32(address_t const& addr) -> std::uint32_t;
auto mcp_u32_to_address(address_t& dest, std::uint32_t const& addr) -> void;
auto mcp_calculate_crc(mcp const& src) -> std::uint8_t;
auto mcp_check_crc(mcp_buffer_t const& buffer) -> bool;
}

#endif  // !SKY_MCP_HPP
