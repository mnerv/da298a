/**
* @file   anim.hpp
* @author Petter Rignell
* brief   Determining startnodes, and calculating delays between animations.
*         If a node is a startnode it should get the delay and then run animation.
*         But after each startnode has ran the animation once, there will be a fixed delay
*         between each animation going forward.
* @date   2022-11-30
**/
namespace sky {

#ifndef ANIM_HPP
#define ANIM_HPP
#include <stdint.h>
#include <topo.hpp>
   
using anim_startnodes = int32_t[sky::node_size];

struct anim {
    bool is_startnode;              //if node is not startnode, don't bother with animation config.
    int8_t initial_animation_delay; //first animation delay
    int8_t animation_delay;         //animation delay after the initial delay
};

//This compares dijkstra for each node and determine starnodes. Outparameter gets all shortest paths in a 2D-array
auto anim_init_startnodes(int32_t dest, anim_startnodes& out_all_startnodes) -> void;

//This calculates both delays depending on the amount of nodes in all animations
auto anim_calc_animation_delays(int32_t nmbr_of_anim_nodes, int32_t nodes_fr_first_to_curr_anim) -> void;

//Initializes animation. This uses the startnode function and the delay function
auto anim_init_animation(int32_t node_id) -> void;

//Startnode: wait animation delay -> LED_ON -> Calculate dijkstra -> Send packet to next node in dijkstra ->
//LED_OFF (turn off with after two sec or something) -> wait another fixed animation delay -> start over with LED_ON
auto anim_run_animations(int32_t node_id) -> void;


#endif // !ANIMATIONS

}





