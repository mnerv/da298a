/**
 * @file   mcp.cpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @author Christian Heisterkamp
 * @brief  sky framework
 * @date   2022-10-19
 *
 * @copyright Copyright (c) 2022
 */
#include "mcp.hpp"
#include "utility.hpp"
#include <cstring>

namespace sky {
auto mcp_make_buffer(mcp_buffer_t& dest, mcp const& src) -> void {
    mcp_buffer_t buffer{};
    // Copy to temporay buffer for later computing CRC-8
    std::memcpy(buffer, &src, mcp_buffer_size);
    buffer[mcp_buffer_size - 1] = crc_8(buffer, mcp_buffer_size - 1);
    // Copy to destination buffer
    std::memcpy(dest, buffer, mcp_buffer_size);
}

auto mcp_make_from_buffer(mcp_buffer_t const& src) -> mcp {
    mcp msg{};
    std::size_t offset = 0;
    msg.type = src[offset++];
    std::memcpy(msg.source,      src + offset, sizeof(address_t));
    offset += sizeof(address_t);
    std::memcpy(msg.destination, src + offset, sizeof(address_t));
    offset += sizeof(address_t);
    std::memcpy(msg.payload,     src + offset, sizeof(payload_t));
    offset += sizeof(payload_t);
    msg.crc = src[offset];
    return msg;
}

auto mcp_address_to_u32(address_t const& addr) -> std::uint32_t {
    uint32_t value = 0;
    value |= static_cast<std::uint32_t>(addr[0]) << 0;
    value |= static_cast<std::uint32_t>(addr[1]) << 8;
    value |= static_cast<std::uint32_t>(addr[2]) << 16;
    return value;
}

auto mcp_u32_to_address(address_t& dest, std::uint32_t const& addr) -> void {
    dest[0] = static_cast<std::uint8_t>((addr & 0x000000FF) >>  0);
    dest[1] = static_cast<std::uint8_t>((addr & 0x0000FF00) >>  8);
    dest[2] = static_cast<std::uint8_t>((addr & 0x00FF0000) >> 16);
}

auto mcp_check_crc(mcp_buffer_t const& buffer) -> bool {
    auto const& packet = mcp_make_from_buffer(buffer);
    auto const& size = mcp_buffer_size - 1;  // skips the crc end
    auto const& crc = crc_8(buffer, size);
    return packet.crc == crc;
}
}
