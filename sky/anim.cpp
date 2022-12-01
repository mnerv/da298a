/**
* @file   anim.cpp
* @author Petter Rignell
* brief   Determining startnodes, and calculating delays between animations.
*         If a node is a startnode it should get the delay and then run animation.
*         But after each startnode has ran the animation once, there will be a fixed delay
*         between each animation going forward.
* @date   2022-11-30
**/

#include "anim.hpp"
#include "string.h"

namespace sky {

    auto anim_init_startnodes(int32_t node_id, int32_t dest, anim_startnodes& out_all_startnodes)->void {
        constexpr size_t len = sky::node_size;
        int32_t all_shortest[len][len];
        int8_t node_counter = 0;
        int8_t shortest_p_length[len];
        sky::topo topology;
        sky::topo_shortest_t output_shortest{};

        //Räkna ut dijkstra för varje nod och placera det i en 2D-array
        for (auto i = 0; i < len; i++) {
            sky::topo_compute_dijkstra(topology, i, dest, output_shortest);
            for (auto j = 0; j < len; j++) {
                if (output_shortest[j] != 0) {
                    node_counter++;
                }
            }
            shortest_p_length[i] = node_counter;
            all_shortest[output_shortest[0] - 1][i] = output_shortest[i];    
        }

        //Sortera alla noders snabbaste väg utefter längd
        int8_t temp = 0;
        int8_t temp2D = 0;

        for (auto i = 0; i < len; i++) {
            for (auto j = 0; j < len; j++) {
                if (shortest_p_length[i] < shortest_p_length[j]) {
                    temp = shortest_p_length[i];
                    shortest_p_length[i] = shortest_p_length[j];
                    for (auto k = 0; k < len; k++) {
                        all_shortest[i][k] = all_shortest[j][k];
                        all_shortest[j][k] = temp;
                    }
                    shortest_p_length[j] = temp;
                }
            }
        }

        int32_t all_animations[len][len];
        int8_t visited_nodes[len];
        int32_t startnodes[len];
        bool visited = true;
       
        //Jämför och skapa en 2D-array för alla animationer
        for (auto i = 0; i < len; i++) {
            for (auto j = 0; j < len; j++) {
                if (all_shortest[i][0] == visited_nodes[j]) {
                    visited = true;
                }
            }
            if (!visited) {
                startnodes[i] = all_shortest[i][0];
                visited_nodes[i] = startnodes[i];
            }     
        } 

        int32_t count_anim_nodes = 0;
        int32_t count_fr_first_node_to_curr = 0;
        bool limit = false;

        for (auto i = 0; i < len; i++) {
            count_anim_nodes += shortest_p_length[startnodes[i]];
            if (startnodes[i] != node_id && limit == false) {
                count_fr_first_node_to_curr += shortest_p_length[startnodes[i]];
            } 
            else if(startnodes[i] == node_id) {
                limit = true;
            }
        }

        anim_calc_animation_delays(count_anim_nodes, count_fr_first_node_to_curr);

        memcpy(out_all_startnodes, startnodes, sky::max_path);
    }


    auto anim_calc_animation_delays(int32_t nmbr_of_anim_nodes, int32_t nodes_fr_first_to_curr_anim)->void {
        //get delays from struct and give them the value of the parameter multiplied by one (a second), or something

    }

    auto anim_init_animation(int32_t node_id, int32_t dest)->void {
        uint8_t len = sky::node_size;
        //sky::anim_startnodes out_all_startnodes{};
        anim_init_startnodes(node_id, dest, out_all_startnodes);
        for (auto i = 0; i < len; i++) {
            if (out_all_startnodes[i] == node_id) {
                anim_run_animations(node_id);
                break;
            }
        }
    }

    auto anim_run_animations(int32_t node_id) ->void{
        
            
            
    }
}