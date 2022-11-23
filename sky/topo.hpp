/**
 * @file   topo.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @author Petter Rignell
 * @brief  Graph topology adjacency matrix and Dijsktra shortest path calculation.
 * @date   2022-11-23
 *
 * @copyright Copyright (c) 2022
 */
#ifndef SKY_TOPO_HPP
#define SKY_TOPO_HPP
#include <stddef.h>

#include "mcp.hpp"

namespace sky {

constexpr size_t max_path  = 16;
constexpr size_t node_size = 16;
using topo_shortest_t = address_t[max_path];

struct topo {
    uint8_t matrix[node_size * node_size];
};

auto topo_index_matrix(topo const& topology, uint32_t row, uint32_t column) -> uint8_t;
auto topo_set_node_link_cost(topo& topology, uint32_t node_id, uint8_t cost) -> void;
auto topo_compute_dijkstra(topo const& topology, uint32_t src, uint32_t destination, topo_shortest_t& shortest_path) -> void;

} // namespace sky

#endif  // !SKY_TOPO_HPP
