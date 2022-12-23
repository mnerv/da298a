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
#include <algorithm>
#include <cstdio>

#include "Arduino.h"
#include "Adafruit_NeoPixel.h"
#include "SoftwareSerial.h"
// #include "ESP8266WiFi.h"

#include "sky.hpp"
#include "control_register.hpp"
#include "porter.hpp"
#include "config_status.hpp"

#define HARDWARE_BAUD 115200
#define SOFTWARE_BAUD 9600

#define RX_PIN       D5
#define TX_PIN       D6
#define LED_PIN      D1
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

//16 = number of nodes
sky::address_t address_set[16];
int32_t neighbour_list[16][4];
size_t index_address_set = 0;
sky::topo topo{};
sky::topo_shortest_t shortestpath{};

uint32_t pixel_time = 0;
uint32_t pixel_interval = 33;
uint32_t config_time = 0;

static ray::control_register control;
static ray::porter porter(RX_PIN, TX_PIN, SOFTWARE_BAUD, control);
static ray::config_status config_status(CONFIG_PIN, control);

static Adafruit_NeoPixel pixel(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);
uint32_t pixel_color[LED_COUNT] {0, 0, 0, 0};

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
    Serial.printf("crc: %02x", mcp.crc);
    Serial.println("}");
}

auto print_packet(ray::packet const& pkt) -> void {
    if (pkt.size != ray::PACKET_DATA_SIZE) return;
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

    pinMode(RX_PIN, INPUT);
    pinMode(TX_PIN, OUTPUT);
    porter.begin();

    pinMode(CONFIG_PIN, INPUT);

    // Setup addressable LEDs
    pixel.begin();
    pixel.clear();
    pixel.setBrightness(16);
    pixel.show();
    delay(1000);
}

void loop() {
    porter.poll();
    auto current = millis();
    //delay(1);

    switch (current_state) {
    case node_state::config: {
        if (current - config_time > 128) {
            config_time = current;
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
                porter.write(packet);
            }
            if (!verified_edges[1]) {
                packet.channel = 1;
                porter.write(packet);
            }
            if (!verified_edges[2]) {
                packet.channel = 2;
                porter.write(packet);
            }
            if (!verified_edges[3]) {
                packet.channel = 3;
                porter.write(packet);
            }
        }

        auto ch0 = porter.read(0);
        auto ch1 = porter.read(1);
        auto ch2 = porter.read(2);
        auto ch3 = porter.read(3);

        // handle_message(ch0);
        // handle_message(ch1);
        // handle_message(ch2);
        // handle_message(ch3);

        print_packet(ch0);
        print_packet(ch1);
        print_packet(ch2);
        print_packet(ch3);

        if (max_config_tries <= 0) {
            current_state = node_state::idle;
        }

        pixel.setPixelColor(0, 0xFF00FF);
        pixel.setPixelColor(1, 0xFF00FF);
        pixel.setPixelColor(2, 0xFF00FF);
        pixel.setPixelColor(3, 0xFF00FF);
    } break;
    case node_state::idle: {
        pixel.setPixelColor(0, 0x00FF00);
        pixel.setPixelColor(1, 0x00FF00);
        pixel.setPixelColor(2, 0x00FF00);
        pixel.setPixelColor(3, 0x00FF00);
    } break;
    case node_state::fire: {
        pixel.setPixelColor(0, 0xFF0000);
        pixel.setPixelColor(1, 0xFF0000);
        pixel.setPixelColor(2, 0xFF0000);
        pixel.setPixelColor(3, 0xFF0000);
    } break;
    }
 
    if (current - pixel_time > pixel_interval) {
        pixel_time = current;
        pixel.show();
    }
}
