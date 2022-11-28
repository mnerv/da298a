/**
 * @file   topo_tests.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @author Petter Rignell
 * @brief  Graph topology adjacency matrix and Dijsktra shortest path calculation.
 * @date   2022-11-23
 *
 * @copyright Copyright (c) 2022
 */
#ifndef TEST_TOPO_TESTS_HPP
#define TEST_TOPO_TESTS_HPP

#include "gtest/gtest.h"

TEST(sky_topo, topo_index_matrix) {
    ASSERT_TRUE(false);
}

TEST(sky_topo, topo_set_node_link_cost) {
    ASSERT_TRUE(false);
}

TEST(sky_topo, topo_compute_dijkstra) {
    sky::topo topology;
    int8_t matrix[6][6] = {
        { 0,1,1,1,-1,-1 },
        { 1,0,1,1,1,-1 },
        { 1,1,0,-1,1,-1 },
        { 1,1,-1,0,1,1 },
        { -1,1,1,1,0,1 },
        { -1,-1,-1,1,1,0 }
    };
    for (size_t i = 0; i < 6; i++) {
        for (size_t j = 0; j < 6; j++) {
            topology.matrix[i][j] = matrix[i][j];
        }
    }

    sky::topo_shortest_t expected_shortest_path = {1,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0};
    sky::topo_shortest_t output_shortest{};

    sky::topo_compute_dijkstra(topology, 1, 6, output_shortest);
    for (auto i = 0; i < 16; i++) {
        EXPECT_EQ(expected_shortest_path[0], output_shortest[0]);
    }
}
#endif  // !TEST_TOPO_TESTS_HPP
