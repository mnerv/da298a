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
#include "mode.hpp"

#define HARDWARE_BAUD 115200
#define SOFTWARE_BAUD 9600

#define RX_PIN       D5
#define TX_PIN       D6
#define LED_PIN      D8

#define SR_CLK_PIN   D4 //Förra D7
#define SR_DATA_PIN  D3 //Förra D4
#define SR_LATCH_PIN D7 //Förra D3

static Adafruit_NeoPixel pixel(1, LED_PIN, NEO_RGB + NEO_KHZ800);
static SoftwareSerial serial(RX_PIN, TX_PIN);

enum class state : uint8_t {
    config,
    idle,
    error,
};
state previous_state = state::config;
state current_state  = state::config;

enum class init_state : uint8_t {
    listen,
    wait_listen,
    done_listen,
    read,
    write,
};
init_state previous_init_state = init_state::listen;
init_state current_init_state  = init_state::listen;

uint64_t start_time = 0;
uint64_t wait_time  = 0;

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

auto set_mode(hw::serial const& ch, hw::mode const& mode) -> void{
    digitalWrite(SR_LATCH_PIN, LOW);
    shiftOut(SR_DATA_PIN, SR_CLK_PIN, MSBFIRST, ch | mode);
    digitalWrite(SR_LATCH_PIN, HIGH);
}

auto config_routine() -> state {
    if (current_init_state == init_state::listen) {
        pixel.setPixelColor(0, 0x6f03fc);
        pixel.show();
        set_mode(hw::serial::ch0, hw::mode::rx);
        serial.listen();
        current_init_state = init_state::wait_listen;
        return state::config;
    }

    if (current_init_state == init_state::wait_listen) {
        if (current_init_state != previous_init_state) {
            wait_time  = random(250, 3000);  // TODO: make this a constant
            start_time = millis();
            previous_init_state = current_init_state;
        } else {
            if (millis() - start_time > wait_time) {
                current_init_state = init_state::done_listen;
            }
            pixel.setPixelColor(0, 0xe303fc);
            pixel.show();
        }
    }

    if (current_init_state == init_state::done_listen) {
        pixel.setPixelColor(0, 0x03dbfc);
        pixel.show();
        previous_init_state = current_init_state;
        if (serial.available() >= static_cast<int>(sky::mcp_buffer_size)) {
            serial.stopListening();
            current_init_state = init_state::read;
        } else {
            current_init_state  = init_state::write;
            set_mode(hw::serial::ch0, hw::mode::tx);
        }
    }

    if (current_init_state == init_state::read) {
        pixel.setPixelColor(0, 0xfcad03);
        pixel.show();
        sky::mcp_buffer_t buffer{};
        auto len = serial.read(buffer, sky::mcp_buffer_size);
        if (len == sky::mcp_buffer_size) {
            sky::mcp mcp = sky::mcp_make_from_buffer(buffer);
            print_mcp(mcp);
            previous_init_state = current_init_state;
            current_init_state  = init_state::write;
        }
    }

    if (current_init_state == init_state::write) {
        pixel.setPixelColor(0, 0xfc4e03);
        pixel.show();
        previous_init_state = current_init_state;
        if (serial.available() >= static_cast<int>(sky::mcp_buffer_size)) {
            current_init_state = init_state::read;
        } else {
            sky::mcp_buffer_t buffer{};
            sky::mcp mcp{ 0, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 };
            sky::mcp_u32_to_address(mcp.source, ESP.getChipId());
            sky::mcp_make_buffer(buffer, mcp);
            serial.write(buffer, sky::mcp_buffer_size);
            serial.flush();
            current_init_state = init_state::listen;
        }
    }

    return state::config;
}

auto idle_routine() -> state {
    return state::idle;
}

auto error_routine() -> state {
    pixel.setPixelColor(0, 0xFF0000);
    pixel.show();
    return state::error;
}

void setup() {
    Serial.begin(HARDWARE_BAUD);
    serial.begin(SOFTWARE_BAUD);

    pinMode(SR_CLK_PIN,  OUTPUT);
    pinMode(SR_DATA_PIN, OUTPUT);
    pinMode(SR_LATCH_PIN, OUTPUT);
    
    pixel.setBrightness(16);
    pixel.begin();
}

void loop() {
    switch (current_state) {
    case state::config:
        current_state = config_routine();
        break;
    case state::idle:
        current_state = idle_routine();
        break;
    case state::error:
        current_state = error_routine();
        break;
    }
}
