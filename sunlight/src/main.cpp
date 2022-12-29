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
bool neighbour_in_fire[ray::MAX_CHANNEL] {false, false, false, false};
bool reset[ray::MAX_CHANNEL] {false, false, false, false};
uint32_t start_time = 0;

//16 = number of nodes
sky::address_t address_set[16];
int32_t neighbour_list[16][4];
size_t index_address_set = 0;
sky::topo topo{};
sky::topo_shortest_t shortestpath{};

uint32_t pixel_time     = 0;
uint32_t pixel_interval = 33;

static ray::control_register control;
static ray::multicom com(RX_PIN, TX_PIN, SOFTWARE_BAUD, control);
static ray::config_status config_status(CONFIG_PIN, control);

static Adafruit_NeoPixel pixel(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

WiFiClient client;

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

auto saveMyEdges(){
    auto index = 0;
    //16 is number of nodes in system.
    for (size_t i = 0; i < 16; i++)
    {
        if (address_set[i][0] == 0 && address_set[i][1] == 0 && address_set[i][2] == 0){
            index = i;
            break;
        }
    }

    for (size_t i = 0; i < 4; i++)
    {
        if (edges[i][0] != 0 && edges[i][1] != 0 && edges[i][2] != 0)
        {
            memcpy(address_set[index], edges[i], sky::address_size);
            index_address_set++;
            index++;
        }
    }

    for (size_t i = 0; i < 4; i++)
    {
        if (edges[i][0] != 0 && edges[i][1] != 0 && edges[i][2] != 0)
        {
            for (size_t j = 0; j < 16; j++)
            {
                if (edges[i][0] == address_set[j][0] && edges[i][1] == address_set[j][1] && edges[i][2] == address_set[j][2])
                {
                    neighbour_list[0][i] = j;
                    break;
                }
            } 
        }else{
            neighbour_list[0][i] = -1;
        }
    }
}

auto updateEdges(sky::mcp mcp){
    //address_set[0] addresser
    //neighbour_list[0][0] grannar till address
    Serial.println(index_address_set);
    sky::address_t mac_addr{
        mcp.payload[0],
        mcp.payload[1],
        mcp.payload[2],
    };

    sky::address_t neighbour1{
    mcp.payload[3],
    mcp.payload[4],
    mcp.payload[5],
    };

    sky::address_t neighbour2{
    mcp.payload[6],
    mcp.payload[7],
    mcp.payload[8],
    };

    sky::address_t neighbour3{
    mcp.payload[9],
    mcp.payload[10],
    mcp.payload[11],
    };

    sky::address_t neighbour4{
    mcp.payload[12],
    mcp.payload[13],
    mcp.payload[14],
    };
    
    auto exist0 = std::find_if(address_set , address_set + 16,
        [&](sky::address_t const& addr){
            return sky::mcp_address_to_u32(addr) == sky::mcp_address_to_u32(mac_addr);
    });
    auto exist1 = std::find_if(address_set , address_set + 16,
        [&](sky::address_t const& addr){
            return sky::mcp_address_to_u32(addr) == sky::mcp_address_to_u32(neighbour1);
    });
    auto exist2 = std::find_if(address_set , address_set + 16,
        [&](sky::address_t const& addr){
            return sky::mcp_address_to_u32(addr) == sky::mcp_address_to_u32(neighbour2);
    });
    auto exist3 = std::find_if(address_set , address_set + 16,
        [&](sky::address_t const& addr){
            return sky::mcp_address_to_u32(addr) == sky::mcp_address_to_u32(neighbour3);
    });
    auto exist4 = std::find_if(address_set , address_set + 16,
        [&](sky::address_t const& addr){
            return sky::mcp_address_to_u32(addr) == sky::mcp_address_to_u32(neighbour4);
    });

    auto srcindex = 0;

    if (exist0 != address_set + 16) {
        srcindex = (size_t)(exist0 - address_set);
        memcpy(neighbour_list[srcindex], mac_addr, sky::address_size);
    } else{
        memcpy(address_set[index_address_set], mac_addr, sky::address_size);
        srcindex = index_address_set;
        index_address_set++;
    }

    if (sky::mcp_address_to_u32(neighbour1) != 0)
    {
        if (exist1 != address_set + 16) {
            auto index = (uint32_t)(exist1 - address_set);
            neighbour_list[srcindex][0] = index;
        }else{
            memcpy(address_set[index_address_set], neighbour1, sky::address_size);
            neighbour_list[srcindex][0] = index_address_set;
            index_address_set++;
        }
    }else{
        neighbour_list[srcindex][0] = -1;
    }
    
    if (sky::mcp_address_to_u32(neighbour2) != 0)
    {
        if (exist2 != address_set + 16) {
            auto index = (uint32_t)(exist2 - address_set);
            neighbour_list[srcindex][1] = index;
        }else{
            memcpy(address_set[index_address_set], neighbour2, sky::address_size);
            neighbour_list[srcindex][1] = index_address_set;
            index_address_set++;
        }
    }else{
        neighbour_list[srcindex][1] = -1;
    }

    if (sky::mcp_address_to_u32(neighbour3) != 0)
    {
        if (exist3 != address_set + 16) {
            auto index = (uint32_t)(exist3 - address_set);
            neighbour_list[srcindex][2] = index;
        }else{
            memcpy(address_set[index_address_set], neighbour3, sky::address_size);
            neighbour_list[srcindex][2] = index_address_set;
            index_address_set++;
        }
    }else{
        neighbour_list[srcindex][2] = -1;
    }

    if (sky::mcp_address_to_u32(neighbour4) != 0)
    {
        if (exist4 != address_set + 16) {
            auto index = (uint32_t)(exist4 - address_set);
            neighbour_list[srcindex][3] = index;
        }else{
            memcpy(address_set[index_address_set], neighbour4, sky::address_size);
            neighbour_list[srcindex][3] = index_address_set;
            index_address_set++;
        }
    }else{
        neighbour_list[srcindex][3] = -1;
    }
}

auto printAddrSetAndNeighbour(){
    for (size_t i = 0; i < 16; i++)
    {
        Serial.printf("\nAddress_set: %02x:%02x:%02x", address_set[i][0], address_set[i][1], address_set[i][2]);
        Serial.printf("\nNeighbour list: %d, %d, %d, %d \n", neighbour_list[i][0], neighbour_list[i][1], neighbour_list[i][2], neighbour_list[i][3]);
    }
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
    //for (size_t i = 0; i < 16; i++)
    //{
        // for (size_t j = 0; j < 16; j++)
        // {
        //     //Show matrix
        //     Serial.print(topo.matrix[i][j] > 0 ? '1' : topo.matrix[i][j] == -1 ? '-' : '0');
        //     Serial.print(topo.matrix[i][j]);
        //     Serial.print(" ");
        // }  
        //Serial.println();
    //}
}


auto printPath(sky::topo_shortest_t const& path){
    Serial.print("\nShortest Path: ");
    for (size_t i = 0; i < 16; i++)
    {
        if (path[i] != 0)
        {
            Serial.printf("%d ", path[i]);
        }    
    }
    Serial.println();
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
    auto channel = packet.channel;

    //If type 0 edges finding
    if (mcp.type == 0) {
        //If ack just verify the edge and save its mac
        if (mcp.payload[0] == 1) {
            memcpy(edges[channel], mcp.source, sky::address_size);
            verified_edges[channel] = true;
            com.clear_buffer(channel);
        } else {
            //If recived from channel not verified just save its MAC
            if (verified_edges[channel] == false)
            {
                memcpy(edges[channel], mcp.source, sky::address_size);
                verified_edges[channel] = true;
            } 

            //Replying with ACK
            memcpy(mcp.destination, mcp.source, sky::address_size);
            sky::mcp_u32_to_address(mcp.source, ESP.getChipId()); 
            mcp.payload[0] = 1;
            sky::mcp_make_buffer(buffer, mcp);
            ray::packet pkt{};
            memcpy(pkt.data, buffer, sky::mcp_buffer_size);
            pkt.size = static_cast<uint8_t>(sky::mcp_buffer_size);
            pkt.channel = channel;
            for (auto i = 0; i < 16; ++i)  // Flood the buffer
                com.write(pkt); 
        } 
    }else if (mcp.type == 1) {
        updateEdges(mcp);
        printAddrSetAndNeighbour();
        createTopo();
        sky::topo_compute_dijkstra(topo, 3, 2, shortestpath);
        printPath(shortestpath);
        
        for (size_t i = 0; i < 4; i++)
        {
            if (packet.channel != i && verified_edges[i] == true)
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
                for (auto i = 0; i < 8; ++i) com.write(pkt);
            }
        } 
        //FIRE
    }else if (mcp.type == 3)
    {
        if (mcp.payload[0] == 1)
        {
            neighbour_in_fire[channel] = true;
            com.clear_buffer(channel);
        }else {
            current_state = node_state::fire;
            sky::mcp_buffer_t buffer{};
            sky::mcp mcp{ 3, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 };
            sky::mcp_u32_to_address(mcp.source, ESP.getChipId());
            memcpy(mcp.destination, edges[channel], sky::address_size);
            sky::mcp_make_buffer(buffer, mcp);
            ray::packet packet{};
            packet.size = static_cast<uint8_t>(sky::mcp_buffer_size);
            for (size_t i = 0; i < ray::MAX_CHANNEL; i++)
            {
                if(channel == i)
                {
                    mcp.payload[0] = 1;
                }
                
                if (verified_edges[i] == true)
                {
                    packet.channel = i;
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
                    memcpy(packet.data, buffer, sky::mcp_buffer_size);

                    for (auto j = 0; j < 16; ++j) { // Flood the buffer
                      com.write(packet);
                    }
                }  
            }   
        }   
    }
};

void loop() {
    com.poll();

    switch (current_state) {
    case node_state::config: {
        pixel.setPixelColor(0, 0xFF0000);

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
            saveMyEdges();
        }
        break;
    }
    case node_state::idle: {
        
        //Show neighbour
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
                if (edges[i][0] != 0)
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
        }

        auto ch0 = com.read(0);
        auto ch1 = com.read(1);
        auto ch2 = com.read(2);
        auto ch3 = com.read(3);

        handle_message(ch0);
        handle_message(ch1);
        handle_message(ch2);
        handle_message(ch3);


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

        if (config_status.is_fire())
        {
            current_state = node_state::fire;

        }
        break;
    }
    case node_state::fire: {
        for (size_t i = 0; i < 4; i++)
        {
            pixel.setPixelColor(i, 0xFF0000);
        }

        for (size_t i = 0; i < ray::MAX_CHANNEL; i++)
        {
            if (neighbour_in_fire[i] == false && verified_edges[i] == true)
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
