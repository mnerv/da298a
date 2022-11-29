/**
 * @file   topo.cpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @author Petter Rignell
 * @author Reem Mohamed
 * @brief  Graph topology adjacency matrix and Dijsktra shortest path calculation.
 * @date   2022-11-23
 *
 * @copyright Copyright (c) 2022
 */
#include "topo.hpp"

namespace sky {
/*
auto topo_index_matrix(topo const& topology, uint32_t row, uint32_t column) -> uint8_t {
    return topology.matrix[row * node_size + column];
}
*/


auto topo_set_node_link_cost(topo& topology, uint32_t node_id, uint32_t endNode_id, int8_t cost) -> topo {
    topology.matrix[node_id][endNode_id] = cost;
    topology.matrix[endNode_id][node_id] = cost;

    return topology;
}

auto topo_compute_dijkstra(topo const& topology, int32_t src, int32_t dest, topo_shortest_t& out_shortest) -> void {
    int32_t prev_node[node_size]{};
    int32_t min_distance[node_size]{};
    int32_t start = src - 1;
    int32_t end = dest - 1;
    int32_t unvisited_nodes[node_size]{};

    //If node is in firemode
    if (topology.matrix[start][start] == -1) {
        memset(out_shortest, 0, max_path);
        return;
    }

    //Initialize unvisited_nodes array and stored nodes array (prev_nodes)
    for (auto i = 1; i < node_size + 1; i++) {
        unvisited_nodes[i - 1] = i;
        if (i != src) {
            min_distance[i - 1] = 255;
        }
    }
    prev_node[start] = start;


    //Get size of unvisited_nodes and check if it's empty
    bool isempty = true;
    int32_t unvisited_nodes_size = 0;

    for (auto i = 0; i < node_size; i++) {
        if (unvisited_nodes[i] != 0) {
            isempty = false;
            unvisited_nodes_size++;
        }
    }

    //Main loop for comparing paths and deciding which path is shortest
    while (!isempty) {
        int32_t min = 255;
        int32_t new_node = -1;

        //Get next node 
        for (auto i = 0; i < unvisited_nodes_size; i++) {
            if (min_distance[unvisited_nodes[i] - 1] < min) {
                new_node = unvisited_nodes[i] - 1;
                min = min_distance[i];
            }
        }

        int32_t current_node = new_node;

        //If there's no link
        if (current_node == -1) {
            break;
        }

        //Remove and sort unvisited nodes, because the current node has been visited
        bool isremoved = false;

        for (auto i = 0; i < unvisited_nodes_size; i++) {
            if (i != unvisited_nodes_size && unvisited_nodes[i] == current_node + 1) {
                unvisited_nodes[i] = 0;
                isremoved = true;
                unvisited_nodes[i] = unvisited_nodes[i + 1];
            }
            else if (i != unvisited_nodes_size && isremoved) {
                unvisited_nodes[i] = unvisited_nodes[i + 1];
            }
        }

        unvisited_nodes_size--;

        //Compare distances and store nodes in prev_node
        bool contains_node = false;

        for (auto i = 0; i < node_size; i++) {
            for (auto j = 0; j < unvisited_nodes_size; j++) {
                if (unvisited_nodes[j] == i + 1) {
                    contains_node = true;
                    break;
                }
            }

            if (contains_node && topology.matrix[current_node][i] > 0) {
                int32_t new_distance = min_distance[current_node] + topology.matrix[current_node][i];

                if (new_distance < min_distance[i]) {
                    min_distance[i] = new_distance;
                    prev_node[i] = current_node;
                }
            }
            contains_node = false;
        }
    }

    //Arrange the path from the node array, prev_node
    int32_t path_arr[node_size]{};
    auto k = end;
    auto itr = 1;
    int32_t node_path_counter = 0;

    while (true) {
        if (prev_node[k] == k) {
            break;
        }
        path_arr[itr] = prev_node[k] + 1;
        node_path_counter++;
        k = prev_node[k];
        itr++;
    }
    path_arr[0] = dest;
    node_path_counter++;

    //If there's no path out (blocked by fire)
    if (src != dest) {
        if (!(topology.matrix[end][path_arr[1] - 1] != -1)) {
            memset(out_shortest, 0, max_path);
            return;
        }
    }

    //Reverse array, src first, dest last
    auto temp = 0;

    for (auto low = 0, high = node_path_counter - 1; low < high; low++, high--) {
        temp = path_arr[low];
        path_arr[low] = path_arr[high];
        path_arr[high] = temp;
    }

    memcpy(out_shortest, path_arr, max_path);
}

}
