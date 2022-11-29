/**
 * @file   topo_tests.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @author Petter Rignell
 * @author Reem Mohamed
 * @brief  Graph topology adjacency matrix and Dijsktra shortest path calculation.
 * @date   2022-11-23
 *
 * @copyright Copyright (c) 2022
 */
#ifndef TEST_TOPO_TESTS_HPP
#define TEST_TOPO_TESTS_HPP

#include "topo.hpp"
#include "gtest/gtest.h"
#include "fmt/format.h"


//TEST(sky_topo, topo_index_matrix) {
  //      ASSERT_TRUE(false);
//}

TEST(sky_topo, topo_set_node_link_cost) {

    sky::topo topology;
    int8_t topology_matrix[5][5] = {
        {0,2,3,1,-1},
        {2,0,7,3,5},
        {3,7,0,-1,6},
        {1,3,-1,0,4},
        {-1,5,6,4,0}
    };
    for (size_t i = 0; i < 5; i++) {
        for (size_t j = 0; j < 5; j++) {
            topology.matrix[i][j] = topology_matrix[i][j];
        }
    }
    topology = sky::topo_set_node_link_cost(topology, 0, 2, 5);
    int32_t output1 = (int32_t) topology.matrix[0][2];
    int32_t output2 = (int32_t) topology.matrix[2][0];

    int32_t expected_value = 5;

    fmt::print("{}\n", output1);
    fmt::print("{}\n", output2);

    EXPECT_EQ(expected_value, output1);
    EXPECT_EQ(expected_value, output2);

}

TEST(sky_topo, topo_compute_dijkstra) {
    ASSERT_TRUE(false);
}

#endif  // !TEST_TOPO_TESTS_HPP
