/**
 * @file   main.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @author Linnéa Mörk
 * @author Isac Pettersson
 * @author Reem Mohamed
 * @brief  sunlight embedded program
 * @date   2022-10-23
 *
 * @copyright Copyright (c) 2022
 */
#include "Arduino.h"
#include "Adafruit_NeoPixel.h"
#include "SoftwareSerial.h"
#include "ESP8266WiFi.h"
#include <algorithm>
#include <numeric>

#include "sky.hpp"
#include "control_register.hpp"
#include "multicom.hpp"
#include "config_status.hpp"

#define HARDWARE_BAUD 115200
#define SOFTWARE_BAUD 9600

#define RX_PIN       D5
#define TX_PIN       D6
#define LED_PIN      D8
#define LED_COUNT    4

#define CONFIG_PIN   A0

#define SR_CLK_PIN   D4  // Shift register clock pin
#define SR_DATA_PIN  D3  // Shift register data pin
#define SR_LATCH_PIN D7  // Shift register latch pin

enum class node_state {
    config,
    idle,
    fire
};

node_state current_state = node_state::config;
int32_t max_config_tries = 255;

sky::address_t edges[ray::MAX_CHANNEL] {};
bool verified_edges[ray::MAX_CHANNEL] {false, false, false, false};
//Needs to be improved
bool neighbour_in_fire[ray::MAX_CHANNEL] {false, false, false, false};
//Needs to be improved
bool reset[ray::MAX_CHANNEL] {false, false, false, false};

sky::address_t exitAddr{};

uint32_t start_time = 0;

//16 = number of nodes
sky::address_t address_set[16]{};
int32_t neighbour_list[16][4]{};
size_t index_address_set = 0;
sky::topo topo{};

bool fire_node = false;

//Chosen path
sky::topo_shortest_t shortestpath{};

//List of all dijkstra paths
sky::topo_shortest_t shorestpathList[16]{};

uint32_t pixel_time     = 0;
uint32_t pixel_interval = 33;

bool is_light_on = false;
bool has_animation_packet = false;

static ray::control_register control;
static ray::multicom com(RX_PIN, TX_PIN, SOFTWARE_BAUD, control);
static ray::config_status config_status(CONFIG_PIN, control);

static Adafruit_NeoPixel pixel(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

auto print_mcp(sky::mcp const& mcp) -> void {
    sky::address_t id{};
    sky::mcp_u32_to_address(id, ESP.getChipId());
    Serial.printf("%02x:%02x:%02x: ", id[0], id[1], id[2]);
    Serial.print("mcp{");
    Serial.printf("type: %02x, ", mcp.type);
    Serial.printf("src: %02x:%02x:%02x, ", mcp.source[0], mcp.source[1], mcp.source[2]);
    Serial.printf("dst: %02x:%02x:%02x, ", mcp.destination[0], mcp.destination[1], mcp.destination[2]);
    Serial.print("data: [");
    for (size_t i = 0; i < sky::payload_size; ++i) {
        Serial.printf("%02x", mcp.payload[i]);
        if (i < sky::payload_size - 1) Serial.print(", ");
    }
    Serial.print("], ");
    Serial.printf("crc: %d", mcp.crc);
    Serial.println("}");
}

auto print_packet(ray::packet const& pkt) -> void {
    if (pkt.size != sky::mcp_buffer_size) return;
    sky::mcp_buffer_t buffer{};
    memcpy(buffer, pkt.data, sky::mcp_buffer_size);
    auto mcp = sky::mcp_make_from_buffer(buffer);
    print_mcp(mcp);
}

auto update_shift_register(uint8_t data) -> void {
    digitalWrite(SR_LATCH_PIN, LOW);
    shiftOut(SR_DATA_PIN, SR_CLK_PIN, MSBFIRST, data);
    digitalWrite(SR_LATCH_PIN, HIGH);
}

auto insert_to_address_set(sky::address_t const& addr) -> void {
    if (sky::mcp_address_to_u32(addr) == 0) return;
    
    //Finds addr in address_set
    auto const it = std::find_if(address_set, address_set + sky::length_of(address_set), [&addr](sky::address_t const& a) {
        return sky::mcp_address_to_u32(addr) == sky::mcp_address_to_u32(a);
    });
    //Not exists insert MAC into address_set.
    if (it == address_set + sky::length_of(address_set)) {
        memcpy(address_set[index_address_set], addr, sky::address_size);
        ++index_address_set;
    }
}

auto saveMyEdges(){
    //Save ONLY OUR edges. If my/neighbours exist in address_set just set them else add them first.
    auto myMAC = ESP.getChipId();
    sky::address_t my_addr;
    sky::mcp_u32_to_address(my_addr, myMAC);
    insert_to_address_set(my_addr);
    //Adds all edges to address_set if not exits
    for (size_t i = 0; i < ray::MAX_CHANNEL; ++i) {
        if (verified_edges[i]) {
            insert_to_address_set(edges[i]);
        }
    }

    for (size_t i = 0; i < ray::MAX_CHANNEL; ++i) {
        if (!verified_edges[i]) continue;

        auto const it = std::find_if(address_set, address_set + sky::length_of(address_set), [&i](sky::address_t const& a) {
            return sky::mcp_address_to_u32(edges[i]) == sky::mcp_address_to_u32(a);
        });
        if (it != address_set + sky::length_of(address_set)){
            //Find index of edge in address_set
            auto const index = std::distance(address_set, it);
            neighbour_list[0][i] = index;
        }
    }
}

auto updateEdges(sky::mcp mcp){
    //address_set[0] addresser
    //neighbour_list[0][0] grannar till address
    sky::address_t node_addr{
        mcp.payload[0],
        mcp.payload[1],
        mcp.payload[2],
    };

    sky::address_t neighbours[ray::MAX_CHANNEL]{
        {
            mcp.payload[3],
            mcp.payload[4],
            mcp.payload[5],
        },
        {
            mcp.payload[6],
            mcp.payload[7],
            mcp.payload[8],
        },
        {
            mcp.payload[9],
            mcp.payload[10],
            mcp.payload[11],
        },
        {
            mcp.payload[12],
            mcp.payload[13],
            mcp.payload[14],
        }
    };

    insert_to_address_set(node_addr);
    for (size_t i = 0; i < ray::MAX_CHANNEL; i++) {
        insert_to_address_set(neighbours[i]);
    }

    auto const current_it = std::find_if(address_set, address_set + sky::length_of(address_set), [&node_addr](sky::address_t const& a) {
        return sky::mcp_address_to_u32(node_addr) == sky::mcp_address_to_u32(a);
    });
    auto const current_index = std::distance(address_set, current_it);

    for (size_t i = 0; i < sky::length_of(neighbours); ++i) {
        if (sky::mcp_address_to_u32(neighbours[i]) == 0) continue;

        auto const it = std::find_if(address_set, address_set + sky::length_of(address_set), [&](sky::address_t const& a) {
            auto const u32_addr = sky::mcp_address_to_u32(a);
            return sky::mcp_address_to_u32(neighbours[i]) == u32_addr;
        });
        if (it != address_set + sky::length_of(address_set)){
            //Find index of edge in address_set
            auto const index = std::distance(address_set, it);
            neighbour_list[current_index][i] = index;
        }
    }

    Serial.print("current: ");
    Serial.printf("%02x:%02x:%02x - ", node_addr[0], node_addr[1], node_addr[2]);
    for (size_t j = 0; j < ray::MAX_CHANNEL; j++){
        Serial.printf("%02x:%02x:%02x", neighbours[j][0], neighbours[j][1], neighbours[j][2]);
        if (j < ray::MAX_CHANNEL - 1) Serial.print(", ");
        else Serial.println();
    }
    Serial.println("exit");
    Serial.printf("%02x:%02x:%02x\n", exitAddr[0], exitAddr[1], exitAddr[2]);
    Serial.println("neighbour list");
    for (size_t i = 0; i < index_address_set; i++) {
        Serial.print(i);
        Serial.print(": ");
        Serial.printf("%02x:%02x:%02x - ", address_set[i][0], address_set[i][1], address_set[i][2]);
        for (size_t j = 0; j < ray::MAX_CHANNEL; j++){
            auto index = neighbour_list[i][j];
            if (index == -1) index = index_address_set;

            // auto const it = std::find_if(address_set, address_set + sky::length_of(address_set), [&](sky::address_t const& a) {
            //     auto const u32_addr = sky::mcp_address_to_u32(a);
            //     return sky::mcp_address_to_u32(neighbours[i]) == u32_addr;
            // });
            // auto const absolute_index = std::distance(address_set, it);
            Serial.printf("%d: %02x:%02x:%02x", index, address_set[index][0], address_set[index][1], address_set[index][2]);
            if (j < ray::MAX_CHANNEL - 1) Serial.print(", ");
            else Serial.println();
        }
    }
}

auto printAddrSetAndNeighbour(){
    for (size_t i = 0; i < 16; i++)
    {
        Serial.printf("\nAddress_set: %02x:%02x:%02x", address_set[i][0], address_set[i][1], address_set[i][2]);
        Serial.printf("\nNeighbour list: %d, %d, %d, %d \n", neighbour_list[i][0], neighbour_list[i][1], neighbour_list[i][2], neighbour_list[i][3]);
    }
    Serial.println();
}

auto createTopo(){
    for (size_t i = 0; i < 16; i++)
    {
        for (size_t j = 0; j < 16; j++)
        {
            topo.matrix[i][j] = -1;
            if (i == j)
                topo.matrix[i][j] = 0;
        }
    }

    for (size_t i = 0; i < 16; i++)
    {
        if (neighbour_list[i][0] != -1)
        {
            topo.matrix[i][neighbour_list[i][0]] = 1;
        }
        if (neighbour_list[i][1] != -1)
        {
            topo.matrix[i][neighbour_list[i][1]] = 1;
        }
        if (neighbour_list[i][2] != -1)

        {
            topo.matrix[i][neighbour_list[i][2]] = 1;
        }
        if (neighbour_list[i][3] != -1)
        {
            topo.matrix[i][neighbour_list[i][3]] = 1;
        } 
    }
}

auto printTopo(){
    for (size_t i = 0; i < 16; i++)
    {
        for (size_t j = 0; j < 16; j++)
        {
            //Show matrix
            Serial.print(topo.matrix[i][j] > 0 ? '1' : topo.matrix[i][j] == -1 ? '-' : '0');
            //Serial.print(topo.matrix[i][j]);
            Serial.print(" ");
        }  
        Serial.println();
    }
}


auto printPath(sky::topo_shortest_t const& path){
    Serial.print("\nAnim Path: ");
    for (size_t i = 0; i < 16; i++)
    {
        if (path[i] != 0)
        {
            Serial.printf("%d ", path[i] - 1);
        }    
    }
    Serial.println();
}

auto savePath(){
    //Should happen in fire instead
        auto exist = std::find_if(address_set , address_set + 16,
        [&](sky::address_t const& addr){
        return sky::mcp_address_to_u32(addr) == sky::mcp_address_to_u32(exitAddr);
        });

        if (exist != address_set + 16)
        {
            auto exit_index = (int32_t)(exist - address_set);
            auto shortestCount = 0;
            for (size_t i = 0; i < sky::length_of(address_set); i++)
            {
                if (sky::mcp_address_to_u32(address_set[i]) != 0)
                {   
                    shortestCount++;
                    //From i to exit
                    sky::topo_shortest_t path{};
                    // + 1 because thats how dijkstra is implemented
                    sky::topo_compute_dijkstra(topo, (int32_t) i + 1, exit_index + 1, path);
                    memcpy(shorestpathList[i], path, sizeof(sky::topo_shortest_t));
                }
            }
            auto maxIndex = 0;
            auto maxLength = 0;
            for (size_t i = 0; i < (size_t) shortestCount; i++)
            {
                auto length = std::accumulate(shorestpathList[i], shorestpathList[i] + sky::max_path, 0,[](auto const& a, auto const& b) {
                    if (b != 0){
                        return a + 1;
                    }
                    return a;
                });
                
                if (length > maxLength)
                {
                    maxLength = length;
                    maxIndex = i;
                }   
            }    
            memcpy(shortestpath, shorestpathList[maxIndex], sizeof(sky::topo_shortest_t));

            Serial.println("Paths in list");
            for (size_t i = 0; i < 5; i++)
            {
                printPath(shorestpathList[i]);
            }
        }
        //resets the list for next calculation
        memset(shorestpathList, 0, sizeof(shorestpathList));
        //END
}

void setup() {
    Serial.begin(HARDWARE_BAUD);

    // Shift register and control register setup
    pinMode(SR_CLK_PIN,  OUTPUT);
    pinMode(SR_DATA_PIN, OUTPUT);
    pinMode(SR_LATCH_PIN, OUTPUT);
    control.set_register(update_shift_register);

    pinMode(CONFIG_PIN, INPUT);

    // Setup addressable LEDs
    pixel.begin();
    pixel.setBrightness(50);
    pixel.show();

    Serial.println();
    for (size_t i = 0; i < 16; i++)
    {
        address_set[i][0] = 0;
        address_set[i][1] = 0;
        address_set[i][2] = 0;
    }
    
    sky::mcp_u32_to_address(address_set[0], ESP.getChipId());
    index_address_set++;

    for (size_t i = 0; i < 16; i++)
    {
        neighbour_list[i][0] = -1;
        neighbour_list[i][1] = -1;
        neighbour_list[i][2] = -1;
        neighbour_list[i][3] = -1;
    }
}

auto handle_message = [](ray::packet const& packet) {
    if (packet.size != sky::mcp_buffer_size) return;
    
    sky::mcp_buffer_t buffer{};
    memcpy(buffer, packet.data, sky::mcp_buffer_size);
    auto mcp = sky::mcp_make_from_buffer(buffer);
    if (!sky::mcp_check_crc(buffer))
    {
        return;
    } 
    auto channel = packet.channel;

    //If type 0 edges finding
    if (mcp.type == 0) {
        //If ack just verify the edge and save its mac
        if (mcp.payload[0] == 1) {
            memcpy(edges[packet.channel], mcp.source, sky::address_size);
            verified_edges[packet.channel] = true;
            saveMyEdges();
        } else if(mcp.payload[0] == 0){
            //If recived from channel not verified just save its MAC
            if (verified_edges[packet.channel] == false && current_state == node_state::idle)
            {
                memcpy(edges[packet.channel], mcp.source, sky::address_size);  
                verified_edges[packet.channel] = true;
                saveMyEdges();  
            }  
                //Replying with ACK (payload[0] == 1)
                sky::mcp_buffer_t buff{};
                sky::mcp ack{ 0, { 0, 0, 0 }, { 0, 0, 0 }, { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 };
                sky::mcp_u32_to_address(ack.source, ESP.getChipId());
                memcpy(ack.destination, mcp.source, sky::address_size);
                sky::mcp_make_buffer(buff, ack);
                ray::packet pkt{};
                memcpy(pkt.data, buff, sky::mcp_buffer_size);
                pkt.size = static_cast<uint8_t>(sky::mcp_buffer_size);
                pkt.channel = packet.channel;
                for (size_t i = 0; i < 16; i++)
                {
                    com.write(pkt);
                }
        } 
        //Topology
    }else if (mcp.type == 1) {
        updateEdges(mcp);
        //Prints addreset and neightbours for every node
        //printAddrSetAndNeighbour();
        createTopo();
        printTopo();
        //sky::topo_compute_dijkstra(topo, 3, 2, shortestpath);
        //printPath(shortestpath);
        
        for (size_t i = 0; i < ray::MAX_CHANNEL; i++)
        {
            if (channel != i && verified_edges[i] == true)
            {
                mcp.destination[0] = edges[i][0];
                mcp.destination[1] = edges[i][1];
                mcp.destination[2] = edges[i][2];
                sky::mcp_make_buffer(buffer, mcp);
                ray::packet pkt{};
                Serial.printf("Sent on CH: %02x ", i);
                print_mcp(mcp);
                Serial.println();
                sky::mcp_make_buffer(buffer, mcp);
                memcpy(pkt.data, buffer, sky::mcp_buffer_size);
                pkt.size = static_cast<uint8_t>(sky::mcp_buffer_size);
                pkt.channel = i;
                for (auto i = 0; i < 16; ++i) com.write(pkt);
            }
        } 
        //Animation
    }else if(mcp.type == 2){
        //What should happen when reciving animation packet (light up 3-2 sek IDK and turn off wait 1 sek repeat)
        if(!config_status.is_exit()){
            auto const it = std::find_if(address_set, address_set + 16, [&mcp](auto const& addr) {
                return sky::mcp_address_to_u32(addr) == sky::mcp_address_to_u32(mcp.source);
            });

            if (it != address_set + sky::length_of(address_set)) {
                sky::address_t next_address{};
                auto const address_index = std::distance(address_set, it);
                for (size_t i = 0; i < sky::length_of(shortestpath); ++i) {
                    if (shortestpath[i] == address_index) {
                        memcpy(next_address, address_set[i + 2], sky::address_size);
                        break;
                    }
                }

                auto const next_channel_it = std::find_if(edges, edges + sky::length_of(edges), [&next_address](auto const& addr) {
                    return sky::mcp_address_to_u32(addr) == sky::mcp_address_to_u32(next_address);
                });
                auto const next_index = std::distance(edges, next_channel_it);

                sky::mcp_buffer_t buffer{};
                sky::mcp mcp{ 2, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 };
                sky::mcp_u32_to_address(mcp.source, ESP.getChipId());
                memcpy(mcp.destination, next_address, sky::address_size);
                sky::mcp_make_buffer(buffer, mcp);
                ray::packet packet{};
                memcpy(packet.data, buffer, sky::mcp_buffer_size);
                packet.size = static_cast<uint8_t>(sky::mcp_buffer_size);
                packet.channel = next_index;
                for (size_t i = 0; i < 8; i++)
                {
                    com.write(packet);
                }
            }
        }

        has_animation_packet = true;
        
        //FIRE
    }else if (mcp.type == 3)
    {
        if (mcp.payload[0] == 1)
        {
            neighbour_in_fire[channel] = true;
            com.clear_buffer(channel);
        }else {
            current_state = node_state::fire;

            Serial.printf("\nAddress on fire: %02x:%02x:%02x\n", mcp.source[0], mcp.source[1], mcp.source[2]);
            //See if node thats on fire exists in address_set
            auto exist = std::find_if(address_set , address_set + 16,
            [&](sky::address_t const& addr){
                return sky::mcp_address_to_u32(addr) == sky::mcp_address_to_u32(mcp.source);
            });

            //If it exists its on fire and needs to be set into firemode (removed)
            if (exist != address_set + 16) {
                auto index = (size_t)(exist - address_set);
                sky::topo_set_node_firemode(topo, index);
            }
            
            ray::packet packet{};
            packet.size = static_cast<uint8_t>(sky::mcp_buffer_size);
            for (size_t i = 0; i < ray::MAX_CHANNEL; i++)
            {
                if(channel == i)
                {
                    mcp.payload[0] = 1;
                    memcpy(mcp.destination, edges[channel], sky::address_size);
                }else {
                    mcp.payload[0] = 0;
                }
                sky::mcp_make_buffer(buffer, mcp);
                
                if (verified_edges[i] == true)
                {
                    packet.channel = i;
                    memcpy(mcp.destination, edges[i], sky::address_size);
                    memcpy(packet.data, buffer, sky::mcp_buffer_size);

                    for (auto j = 0; j < 16; ++j) { // Flood the buffer
                        com.write(packet);
                    }
                }   
            }
        }  
        //RESET 
    }else if (mcp.type == 4) {
        if (mcp.payload[0] == 1)
        {
            neighbour_in_fire[channel] = false;
            com.clear_buffer(channel);
        }else{ 
            current_state = node_state::idle;

            for (size_t i = 0; i < ray::MAX_CHANNEL; i++)
            {
                sky::mcp_buffer_t buffer{};
                sky::mcp mcp{ 4, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 };
                sky::mcp_u32_to_address(mcp.source, ESP.getChipId());
                sky::mcp_make_buffer(buffer, mcp);
                ray::packet packet{};
            
                packet.size = static_cast<uint8_t>(sky::mcp_buffer_size);
                
                if(channel == i)
                {
                    mcp.payload[0] = 1;   
                }

                if (verified_edges[i] == true)
                {
                    packet.channel = i;
                    

                    for (auto j = 0; j < 16; ++j) { // Flood the buffer
                      com.write(packet);
                    }
                }  
            }   
        }  
        //Exit broadcast 
    }else if(mcp.type == 5){ 
        memcpy(exitAddr, mcp.source, sky::address_size);
        for (size_t i = 0; i < ray::MAX_CHANNEL; i++)
        {
            if (verified_edges[i] == true && channel != i)
            {
                ray::packet packet{};
                packet.size = static_cast<uint8_t>(sky::mcp_buffer_size);
                packet.channel = i;
                memcpy(mcp.destination, edges[i], sky::address_size);
                memcpy(packet.data, buffer, sky::mcp_buffer_size);
                for (size_t j = 0; j < 16; j++)
                {
                    com.write(packet);
                }
            }
        }    
    }
};

void loop() {
    com.poll();

    switch (current_state) {
    case node_state::config: {

        for (size_t i = 0; i < ray::MAX_CHANNEL; i++)
        {
            pixel.setPixelColor(i, 0xFFFF00);
        }

        if (millis() - start_time > 125) {
            start_time = millis();
            --max_config_tries;

            sky::mcp_buffer_t buffer{};
            sky::mcp mcp{ 0, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 };
            sky::mcp_u32_to_address(mcp.source, ESP.getChipId());
            sky::mcp_make_buffer(buffer, mcp);
            ray::packet packet{};
            memcpy(packet.data, buffer, sky::mcp_buffer_size);
            packet.size = static_cast<uint8_t>(sky::mcp_buffer_size);

            if (!verified_edges[0]) {
                packet.channel = 0;
                com.write(packet);
            }
            if (!verified_edges[1]) {
                packet.channel = 1;
                com.write(packet);
            }
            if (!verified_edges[2]) {
                packet.channel = 2;
                com.write(packet);
            }
            if (!verified_edges[3]) {
                packet.channel = 3;
                com.write(packet);
            }

            
            if (config_status.is_exit())
            {
                sky::mcp_u32_to_address(exitAddr, ESP.getChipId());
                sky::mcp_buffer_t buffer{};
                sky::mcp mcp{ 5, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 };
                sky::mcp_u32_to_address(mcp.source, ESP.getChipId());
                sky::mcp_make_buffer(buffer, mcp);
                ray::packet packet{};
                for (size_t i = 0; i < ray::MAX_CHANNEL; i++)
                {
                    if (verified_edges[i])
                    {
                        memcpy(mcp.destination, edges[i], sky::address_size);
                        memcpy(packet.data, buffer, sky::mcp_buffer_size);
                        packet.size = static_cast<uint8_t>(sky::mcp_buffer_size);
                        packet.channel = i;
                        for (size_t j = 0; j < 16; j++)
                        {
                            com.write(packet);
                        }
                        
                    }       
                }          
            }
        }   

        auto ch0 = com.read(0);
        auto ch1 = com.read(1);
        auto ch2 = com.read(2);
        auto ch3 = com.read(3);

        handle_message(ch0);
        handle_message(ch1);
        handle_message(ch2);
        handle_message(ch3);

        print_packet(ch0);
        print_packet(ch1);
        print_packet(ch2);
        print_packet(ch3);

        if (max_config_tries == 0) {
            int32_t edge_count = 0;
            for (auto i = 0; i < 4; ++i)
                if (verified_edges[i]) ++edge_count;

            Serial.printf("\n\nconnected edges: %d, [", edge_count);
            for (auto i = 0; i < 4; ++i) {
                Serial.printf("%02x:%02x:%02x", edges[i][0], edges[i][1], edges[i][2]);
                if (i < 4 - 1) Serial.print(", ");
                else Serial.println("]\n");
            }
            start_time = 0;
            current_state = node_state::idle;
        }
        break;
    }
    case node_state::idle: {
        
        //Show neighbours
        for (size_t i = 0; i < 4; i++)
        {
            if (verified_edges[i])
            {
                pixel.setPixelColor(i, 0x00FF00);
            } else{
                pixel.setPixelColor(i, 0x000000);
            }
        }
        
        if (millis() - start_time > 125) {
            start_time = millis();

            for (size_t i = 0; i < 4; i++)
            {
                if (sky::mcp_address_to_u32(edges[i]) != 0 && verified_edges[i] == true)
                {
                    sky::mcp_buffer_t buffer{};
                    sky::mcp mcp{ 1, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 };
                    sky::mcp_u32_to_address(mcp.source, ESP.getChipId());
                    mcp.destination[0] = edges[i][0];
                    mcp.destination[1] = edges[i][1];
                    mcp.destination[2] = edges[i][2];
                    for (size_t i = 0; i < 3; i++)
                    {
                        mcp.payload[i] = mcp.source[i];
                    }
                    for (size_t i = 3; i < 6; i++)
                    {
                        mcp.payload[i] = edges[0][i-3];
                        mcp.payload[i+3] = edges[1][i-3];
                        mcp.payload[i+6] = edges[2][i-3];   
                        mcp.payload[i+9] = edges[3][i-3];
                    }
                    //Serial.println("Sent: ");
                    //print_mcp(mcp);
                    sky::mcp_make_buffer(buffer, mcp);

                    ray::packet pkt{};
                    memcpy(pkt.data, buffer, sky::mcp_buffer_size);
                    pkt.size = static_cast<uint8_t>(sky::mcp_buffer_size);
                    pkt.channel = i;
                    com.write(pkt);
                } 
            }


            for (size_t i = 0; i < ray::MAX_CHANNEL; i++)
            {
                if (neighbour_in_fire[i] == true && verified_edges[i] == true)
                {
                    sky::mcp_buffer_t buffer{};
                    sky::mcp mcp{ 4, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 };
                    sky::mcp_u32_to_address(mcp.source, ESP.getChipId());
                    sky::mcp_make_buffer(buffer, mcp);
                    ray::packet packet{};
                    memcpy(packet.data, buffer, sky::mcp_buffer_size);
                    packet.size = static_cast<uint8_t>(sky::mcp_buffer_size);
                    packet.channel = i;
                    com.write(packet);      
                }
            }   

        }
        auto ch0 = com.read(0);
        auto ch1 = com.read(1);
        auto ch2 = com.read(2);
        auto ch3 = com.read(3);

        handle_message(ch0);
        handle_message(ch1);
        handle_message(ch2);
        handle_message(ch3);

        if (config_status.is_fire())
        {
            current_state = node_state::fire;
            fire_node = true;
            savePath();
            for (size_t i = 0; i < 4; i++)
            {
                pixel.setPixelColor(i, 0xFF0000);
            }
        }
        break;
    }
    case node_state::fire: {

        for (size_t i = 0; i < ray::MAX_CHANNEL; i++)
        {
            if (neighbour_in_fire[i] == false && verified_edges[i] == true && fire_node == true)
            {
                sky::mcp_buffer_t buffer{};
                sky::mcp mcp{ 3, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 };
                sky::mcp_u32_to_address(mcp.source, ESP.getChipId());
                sky::mcp_make_buffer(buffer, mcp);
                ray::packet packet{};
                memcpy(packet.data, buffer, sky::mcp_buffer_size);
                packet.size = static_cast<uint8_t>(sky::mcp_buffer_size);
                packet.channel = i;
                com.write(packet);    
            }     
        }  


        if (millis() - start_time > 2000)
        {
            start_time = millis();

            //Check if first index in shortestPath is mine. 0 in address_set is always me.
            if(shortestpath[0] == 0){
                for (size_t i = 0; i < ray::MAX_CHANNEL; i++)
                {
                    if (is_light_on)
                        pixel.setPixelColor(i, 0x0011ff);
                    else
                        pixel.setPixelColor(i, 0x000000);
                }
                is_light_on = !is_light_on;

                sky::address_t next_address;
                memcpy(next_address, address_set[shortestpath[1]], sky::address_size);

                auto const next_channel_it = std::find_if(edges, edges + sky::length_of(edges), [&next_address](auto const& addr) {
                    return sky::mcp_address_to_u32(addr) == sky::mcp_address_to_u32(next_address);
                });
                auto const next_index = std::distance(edges, next_channel_it);

                sky::mcp_buffer_t buffer{};
                sky::mcp mcp{ 2, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 };
                sky::mcp_u32_to_address(mcp.source, ESP.getChipId());
                sky::mcp_make_buffer(buffer, mcp);
                ray::packet packet{};
                memcpy(packet.data, buffer, sky::mcp_buffer_size);
                packet.size = static_cast<uint8_t>(sky::mcp_buffer_size);
                packet.channel = next_index;
                com.write(packet);
            } else if (has_animation_packet) {
                for (size_t i = 0; i < ray::MAX_CHANNEL; i++)
                {
                    if (is_light_on)
                        pixel.setPixelColor(i, 0x0011ff);
                    else
                        pixel.setPixelColor(i, 0x000000);
                }
                is_light_on = !is_light_on;
            }
            
            //printAddrSetAndNeighbour();
            printTopo();
            printPath(shortestpath);
            savePath();
            
        }  

        if (config_status.is_reset())
        {
            current_state = node_state::idle;
        }

        auto ch0 = com.read(0);
        auto ch1 = com.read(1);
        auto ch2 = com.read(2);
        auto ch3 = com.read(3);

        handle_message(ch0);
        handle_message(ch1);
        handle_message(ch2);
        handle_message(ch3);

        break;
    }
    }

    if (millis() - pixel_time > pixel_interval) {
        pixel_time = millis();
        pixel.show();
    }
}
