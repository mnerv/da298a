/**
 * @file   topo.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @author Petter Rignell
 * @author Reem Mohamed
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
using topo_shortest_t = int32_t[max_path];

struct topo {
    int8_t matrix[node_size][node_size];
};

auto topo_set_node_link_cost(topo& topology, uint32_t node_id, uint32_t endNode_id, int8_t cost) -> topo;
auto topo_set_node_firemode(topo& topology, uint32_t node_id) -> void;
auto topo_compute_dijkstra(topo const& topology, int32_t src, int32_t dest, topo_shortest_t& out_shortest) -> void;
} // namespace sky

#endif  // !SKY_TOPO_HPP
