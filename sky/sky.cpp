/**
 * @file   sky.cpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  sky framework
 * @date   2022-10-19
 *
 * @copyright Copyright (c) 2022
 */
#include "sky.hpp"
#include <cstring>

namespace sky {
auto make_mcp_buffer(mcp_buffer& dest, mcp const& src) -> void {
    memcpy(dest, &src, mcp_size);
}

auto make_mcp(mcp_buffer const& src) -> mcp {
    mcp msg{};
    size_t offset = 0;
    msg.type = src[offset++];
    memcpy(msg.source,      src + offset, sizeof(address_t));
    offset += sizeof(address_t);
    memcpy(msg.destination, src + offset, sizeof(address_t));
    offset += sizeof(address_t);
    memcpy(msg.payload,     src + offset, sizeof(payload_t));
    offset += sizeof(payload_t);
    msg.crc = src[offset];
    return msg;
}

auto hello() -> char const* { return "sky framework"; }
}
