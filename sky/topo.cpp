/**
 * @file   topo.cpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @author Petter Rignell
 * @brief  Graph topology adjacency matrix and Dijsktra shortest path calculation.
 * @date   2022-11-23
 *
 * @copyright Copyright (c) 2022
 */
#include "topo.hpp"

namespace sky {
auto topo_index_matrix(topo const& topology, uint32_t row, uint32_t column) -> uint8_t {
    return topology.matrix[row * node_size + column];
}

auto topo_set_node_link_cost([[maybe_unused]]topo& topology, [[maybe_unused]]uint32_t node_id, [[maybe_unused]]uint8_t cost) -> void {

}

auto topo_compute_dijkstra([[maybe_unused]]topo const& topology, [[maybe_unused]]uint32_t src, [[maybe_unused]]uint32_t dest, [[maybe_unused]]topo_shortest_t& out_shortest) -> void {

}
}
