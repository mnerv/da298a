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
#include "control_register.hpp"
#include "multicom.hpp"

#define HARDWARE_BAUD 115200
#define SOFTWARE_BAUD 9600

#define RX_PIN       D5
#define TX_PIN       D6
#define LED_PIN      D8
#define LED_COUNT    4

#define SR_CLK_PIN   D4  // Shift register clock pin
#define SR_DATA_PIN  D3  // Shift register data pin
#define SR_LATCH_PIN D7  // Shift register latch pin

#define BUTTON_FIRE D1
#define BUTTON_RESET D2

enum class node_state {
    config,
    idle,
    fire
};

node_state current_state = node_state::config;
int32_t max_config_tries = 255;

sky::address_t edges[ray::MAX_CHANNEL] {};
bool verified_edges[ray::MAX_CHANNEL] {false, false, false, false};
uint32_t start_time = 0;

uint32_t pixel_time     = 0;
uint32_t pixel_interval = 33;

static ray::control_register control;
static ray::multicom com(RX_PIN, TX_PIN, SOFTWARE_BAUD, control);
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

void setup() {
    Serial.begin(HARDWARE_BAUD);

    // Shift register and control register setup
    pinMode(SR_CLK_PIN,  OUTPUT);
    pinMode(SR_DATA_PIN, OUTPUT);
    pinMode(SR_LATCH_PIN, OUTPUT);
    control.set_register(update_shift_register);

    // Buttons
    pinMode(BUTTON_FIRE, INPUT_PULLUP);
    pinMode(BUTTON_RESET, INPUT);

    // Setup addressable LEDs
    pixel.begin();
    pixel.setBrightness(16);
    pixel.show();
}

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


        auto handle_message = [](ray::packet const& packet) {
            if (packet.size != sky::mcp_buffer_size) return;
            sky::mcp_buffer_t buffer{};
            memcpy(buffer, packet.data, sky::mcp_buffer_size);
            auto mcp = sky::mcp_make_from_buffer(buffer);
            auto channel = packet.channel;

            if (mcp.type == 0) {
                if (mcp.payload[0] == 1) {
                    memcpy(edges[channel], mcp.source, sky::address_size);
                    verified_edges[channel] = true;
                } else {
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
            }
        };

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

        if (max_config_tries == 0) current_state = node_state::idle;
        break;
    }
    case node_state::idle: {
        pixel.setPixelColor(0, 0x00FF00);
        if (millis() - start_time > 250) {
            start_time = millis();
            int32_t edge_count = 0;
            for (auto i = 0; i < 4; ++i)
                if (verified_edges[i]) ++edge_count;

            Serial.printf("connected edges: %d, [", edge_count);
            for (auto i = 0; i < 4; ++i) {
                Serial.printf("%02x:%02x:%02x", edges[i][0], edges[i][1], edges[i][2]);
                if (i < 4 - 1) Serial.print(", ");
                else Serial.println("]");
            }
        }
        break;
    }
    case node_state::fire: {
        break;
    }
    }

    if (millis() - pixel_time > pixel_interval) {
        pixel_time = millis();
        pixel.show();
    }
}
