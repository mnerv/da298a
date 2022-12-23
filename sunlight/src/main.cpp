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
uint32_t start_time = 0;

//16 = number of nodes
sky::address_t address_set[16];
int32_t neighbour_list[16][4];
size_t index_address_set = 0;
sky::topo topo{};
sky::topo_shortest_t shortestpath{};

ray::timer<uint32_t> main_timer(125);
ray::timer<uint32_t> pixel_timer(33);
ray::timer<uint32_t> config_timer(3'000);

static ray::control_register control;
static ray::porter porter(RX_PIN, TX_PIN, SOFTWARE_BAUD, control);
static ray::config_status config_status(CONFIG_PIN, control);

static Adafruit_NeoPixel pixel(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

auto update_shift_register(uint8_t data) -> void {
    digitalWrite(SR_LATCH_PIN, LOW);
    shiftOut(SR_DATA_PIN, SR_CLK_PIN, MSBFIRST, data);
    digitalWrite(SR_LATCH_PIN, HIGH);
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

    pinMode(RX_PIN, INPUT);
    pinMode(TX_PIN, OUTPUT);
    porter.begin();

    pinMode(CONFIG_PIN, INPUT);

    // Setup addressable LEDs
    pixel.begin();
    pixel.clear();
    pixel.setBrightness(16);
    pixel.show();
    config_timer.reset();
}

void loop() {
    porter.poll();
    auto current = millis();
    main_timer.update(current);
    pixel_timer.update(current);
    // config_timer.update(current);

    if (main_timer.expired()) {
        main_timer.reset();

        ray::packet packet{};
        for (size_t i = 0; i < 15; i++)
        {
            packet.data[i] = (uint8_t) i;
        }
        packet.size = 15;
        packet.channel = 0;
        porter.write(packet);
        // packet.channel = 1;
        // porter.write(packet);
        // packet.channel = 2;
        // porter.write(packet);
        // packet.channel = 3;
        // porter.write(packet);
    }

    auto print_msg = [&current](ray::packet const& packet) {
        if (packet.size == 0) return;
        static uint32_t mesage_time = 0;
        // Serial.printf("time: %.3fs, delta: %.3fs - ", float(current) / 1000.0f, float(current - mesage_time) / 1000.0f);
        // time (s), delta (s), message (hex), size (byte)
        Serial.printf("%.3f, %.3f, ", float(current) / 1000.0f, float(current - mesage_time) / 1000.0f);
        mesage_time = current;
        for (std::size_t i = 0; i < packet.size; ++i) {
            Serial.printf("%02x ", packet.data[i]);
        }
        Serial.printf(", %d\n", packet.size);
    };

    auto ch0 = porter.read(0);
    print_msg(ch0);

    // switch (current_state) {
    // case node_state::config: {
    //     if (config_timer.expired()) {
    //         config_timer.reset();
    //         current_state = node_state::idle;
    //     }
    //     for (size_t i = 0; i < LED_COUNT; ++i) {
    //         pixel.setPixelColor(i, 0xFFFF00);
    //     }
    // } break;
    // case node_state::idle: {
    //     for (size_t i = 0; i < LED_COUNT; ++i) {
    //         pixel.setPixelColor(i, 0x00FF00);
    //     }
    //     if (config_status.is_fire())
    //         current_state = node_state::fire;
    // } break;
    // case node_state::fire: {
    //     for (size_t i = 0; i < LED_COUNT; ++i) {
    //         pixel.setPixelColor(i, 0xFF0000);
    //     }
    //     if (config_status.is_reset())
    //         current_state = node_state::idle;
    // } break;
    // }
    for (uint16_t i = 0; i < LED_COUNT; ++i) {
        pixel.setPixelColor(i, 0xFF00FF);
    }

    if (pixel_timer.expired()) {
        pixel_timer.reset();
        pixel.show();
    }
}
