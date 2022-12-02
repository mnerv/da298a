/**
 * @file   main.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @author Linnéa Mörk
 * @brief  sunlight embedded program
 * @date   2022-10-23
 *
 * @copyright Copyright (c) 2022
 */
#include "Arduino.h"
#include "Adafruit_NeoPixel.h"
#include "SoftwareSerial.h"

#include "sky.hpp"

#define HARDWARE_BAUD 115200
#define SOFTWARE_BAUD 9600

#define RX_PIN       D5
#define TX_PIN       D6
#define LED_PIN      D8

#define BUTTON_FIRE D1
#define BUTTON_RESET D2

#define SR_CLK_PIN   D4
#define SR_DATA_PIN  D3
#define SR_LATCH_PIN D7

// Communication channels with multiplexer CD4052B
constexpr uint8_t COM_CHANNEL[] {
    0b0000'0000,  // CH0 (CD4052B - X0, Y0)
    0b0000'0001,  // CH1 (CD4052B - X1, Y1)
    0b0000'0010,  // CH2 (CD4052B - X2, Y2)
    0b0000'0011,  // CH3 (CD4052B - X3, Y3)
};
constexpr uint8_t COM_RX_MODE = 0b0000'0000;  // MAX485 - Receive mode
constexpr uint8_t COM_TX_MODE = 0b0000'0100;  // MAX485 - Transmit mode
constexpr uint8_t COM_MAX_CH  = sky::length_of(COM_CHANNEL);  // Number of communication channels 

constexpr uint32_t WAIT_COLOR = 0xFF0000;
constexpr uint32_t RX_COLOR   = 0x00FF00;
constexpr uint32_t TX_COLOR   = 0x0000FF; 

static Adafruit_NeoPixel pixel(2, LED_PIN, NEO_RGB + NEO_KHZ800);
static SoftwareSerial soft(RX_PIN, TX_PIN);

// Hardware specific functions and data structures
namespace hdw {
// Data packet container for sending and receiving
struct packet {
    uint8_t    ch;        // Communication channel
    size_t     size{};    // Size of data
    uint8_t    data[32];  // Data buffer
};

enum class node_state{
    config,
    idle,
    fire
};

} // namespace hdw


enum class com_state : uint8_t {
    receive,
    wait,
    done,
};

uint8_t  com_channel = 0;
uint64_t last_channel_switch     = 0;
uint64_t channel_switch_interval = 35;
com_state previous_com_state = com_state::receive;
com_state current_com_state  = com_state::receive;

hdw::node_state current_state = hdw::node_state::config;
bool verified[COM_MAX_CH] {false};
uint8_t max_attempts[COM_MAX_CH] {255, 255, 255, 255};

sky::address_t edges[COM_MAX_CH] {};

uint64_t start_time = 0;

sky::queue<hdw::packet, 16> packet_queue[COM_MAX_CH]{};

auto send_message(uint8_t channel, sky::mcp const& mcp) -> void {
    hdw::packet packet{};
    packet.ch = COM_CHANNEL[channel];
    packet.size = sky::mcp_buffer_size;
    sky::mcp_buffer_t buffer{};
    sky::mcp_make_buffer(buffer, mcp);
    memcpy(packet.data, buffer, sky::mcp_buffer_size);
    packet_queue[channel].enq(packet);
}

auto print_mcp(sky::mcp const& mcp) -> void {
    sky::address_t id{};
    sky::mcp_u32_to_address(id, ESP.getChipId());
    Serial.printf("%02x:%02x:%02x: ", id[0], id[1], id[2]);
    Serial.print("sky::mcp{");
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

auto on_message(hdw::packet e) -> void {
    if (e.size != sky::mcp_buffer_size) return;
    sky::mcp_buffer_t buffer{};
    memcpy(buffer, e.data, sky::mcp_buffer_size);
    auto mcp = sky::mcp_make_from_buffer(buffer);
    //print_mcp(mcp);
    switch (e.ch) {
    case COM_CHANNEL[0]:
        if (mcp.type == 0)
        {
            if (mcp.payload[0] == 1)
            {
                memcpy(edges[0], mcp.source, sky::address_size);
                verified[0] = true;
            }else{
                memcpy(mcp.destination, mcp.source, sky::address_size);
                sky::mcp_u32_to_address(mcp.source, ESP.getChipId()); 
                mcp.payload[0] = 1;
                send_message(0, mcp);
            } 
        }
        
        break;
    case COM_CHANNEL[1]:
        if (mcp.type == 0)
        {
            if (mcp.payload[0] == 1)
            {
                memcpy(edges[1], mcp.source, sky::address_size);
                verified[1] = true;
            }else{
                memcpy(mcp.destination, mcp.source, sky::address_size);
                sky::mcp_u32_to_address(mcp.source, ESP.getChipId()); 
                mcp.payload[0] = 1;
                send_message(1, mcp);
            } 
        }
        break;
    case COM_CHANNEL[2]:
        if (mcp.type == 0)
        {
            if (mcp.payload[0] == 1)
            {
                memcpy(edges[2], mcp.source, sky::address_size);
                verified[2] = true;
            }else{
                memcpy(mcp.destination, mcp.source, sky::address_size);
                sky::mcp_u32_to_address(mcp.source, ESP.getChipId()); 
                mcp.payload[0] = 1;
                send_message(2, mcp);
            } 
        }
        break;
    case COM_CHANNEL[3]:
        if (mcp.type == 0)
        {
            if (mcp.payload[0] == 1)
            {
                memcpy(edges[3], mcp.source, sky::address_size);
                verified[3] = true;
            }else{
                memcpy(mcp.destination, mcp.source, sky::address_size);
                sky::mcp_u32_to_address(mcp.source, ESP.getChipId()); 
                mcp.payload[0] = 1;
                send_message(3, mcp);
            } 
        }
        break;
    default:
        // Serial.print("UNKNOWN CHANNEL: ");
        break;
    }

    if (e.size == sky::mcp_buffer_size) {
        
    } else {
        // Serial.print("Unknown packet: [");
        // for (size_t i = 0; i < e.size; ++i) {
        //     Serial.printf("%02x", e.data[i]);
        //     if (i < e.size - 1) Serial.print(" ");
        // }
        // Serial.println("]");
    }
}

auto select_channel(uint8_t channel, uint8_t mode) -> void {
    digitalWrite(SR_LATCH_PIN, LOW);
    shiftOut(SR_DATA_PIN, SR_CLK_PIN, MSBFIRST, channel | mode);
    digitalWrite(SR_LATCH_PIN, HIGH);
}

auto packet_event_loop() -> void {
    if (current_com_state == com_state::receive) {
        select_channel(COM_CHANNEL[com_channel], COM_RX_MODE);
        soft.begin(SOFTWARE_BAUD);
        soft.listen();
        current_com_state = com_state::wait;
    }

    // Waiting packets from the current channel state
    if (current_com_state == com_state::wait) {
        pixel.setPixelColor(0, WAIT_COLOR);
        pixel.show();
        if (current_com_state != previous_com_state) {
            last_channel_switch = millis();
            channel_switch_interval = random(16, 66);  // 60 Hz - 15 Hz wait time between channel switches
        } else {
            if (millis() - last_channel_switch > channel_switch_interval)
                current_com_state = com_state::done;
        }
    }

    if (current_com_state == com_state::done) {
        if (soft.available()) {
            pixel.setPixelColor(0, RX_COLOR);
            pixel.show();
            // Read data from serial from the current channel
            // TODO: Check if data length is less than packet data size
            hdw::packet packet{};
            packet.ch = com_channel;
            packet.size = soft.available();
            soft.read(packet.data, packet.size);
            on_message(packet);
        } else if(!packet_queue[com_channel].empty()) {
           pixel.setPixelColor(0, TX_COLOR);
           pixel.show();
           // No data received, try to transmit data in current channel
           auto const& packet = packet_queue[com_channel].deq();
           soft.flush();
           soft.stopListening();
           select_channel(packet.ch, COM_TX_MODE);
           soft.write(packet.data, packet.size);
           soft.flush();
           // Serial.printf("Send packet on channel %d\n", com_channel);
        }
        // Switch to next channel and wait for data
        current_com_state = com_state::receive;
        com_channel = (com_channel + 1) % COM_MAX_CH;
        soft.end();
    }
    previous_com_state = current_com_state;
}

void setup() {
    Serial.begin(HARDWARE_BAUD);

    //Communication
    pinMode(SR_CLK_PIN,  OUTPUT);
    pinMode(SR_DATA_PIN, OUTPUT);
    pinMode(SR_LATCH_PIN, OUTPUT);

    //Buttons
    pinMode(BUTTON_FIRE, INPUT_PULLUP);
    pinMode(BUTTON_RESET, INPUT);

    pixel.begin();
    pixel.setBrightness(16);
    pixel.show();
}

void loop() {

    packet_event_loop();  // Update packet event loop state machine

    switch (current_state)
    {
    case hdw::node_state::config:{
        sky::mcp_buffer_t buffer{};
        sky::mcp mcp{ 0, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 };
        sky::mcp_u32_to_address(mcp.source, ESP.getChipId());
        sky::mcp_make_buffer(buffer, mcp);
        if (verified[0] == false && max_attempts[0] > 0)
        {
            send_message(0, mcp);
            max_attempts[0]--;
        }
        if (verified[1] == false && max_attempts[1] > 0)
        {
            send_message(1, mcp);
            max_attempts[1]--;
        }
        if (verified[2] == false && max_attempts[2] > 0)
        {
            send_message(2, mcp);
            max_attempts[2]--;
        }
        if (verified[3] == false && max_attempts[3] > 0)
        {
            send_message(3, mcp);
            max_attempts[3]--;
        }
        if (max_attempts[0] == 0 && max_attempts[1] == 0 && max_attempts[2] == 0 && max_attempts[3] == 0)
        {
            current_state = hdw::node_state::idle;
        }
        break;
    }

    case hdw::node_state::idle:{
        //TODO - Send / recive topology
        pixel.setPixelColor(1, 0, 0, 255);
        pixel.show();
        if (digitalRead(BUTTON_FIRE) == LOW)
        {
            current_state = hdw::node_state::fire;
        }

        break;
    }
        

    case hdw::node_state::fire:{
        pixel.setPixelColor(1, 255, 0, 0);
        pixel.show();
        break;
    }
    
    default:
        break;
    }

    
    /*
    // Send test message every interval
    if (millis() - start_time > 125) {
        start_time = millis();
        sky::mcp_buffer_t buffer{};
        sky::mcp mcp{ 0, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 };
        sky::mcp_u32_to_address(mcp.source, ESP.getChipId());
        sky::mcp_make_buffer(buffer, mcp);
        send_message(0, mcp);
        send_message(1, mcp);
        send_message(2, mcp);
        send_message(3, mcp);
    }*/
}
