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

#define MSG_REQUEST 0b01010101
#define MSG_REQUEST_ACK 0b10101010

static Adafruit_NeoPixel pixel(1, LED_PIN, NEO_RGB + NEO_KHZ800);
static SoftwareSerial serial(RX_PIN, TX_PIN);

hw::serial current_channel = hw::serial::ch0;
hw::mode current_mode = hw::mode::rx;

uint8_t cfg_count = 5;
uint8_t temp = 5;
sky::address_t id;


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

auto handle_packet() -> void {
    serial.listen();
    auto deltaTime = (unsigned long) 2000;
    auto startTime = millis();
    while (millis() - startTime < deltaTime){
        if (serial.available() > 23)
        {
            sky::mcp_buffer_t buffer{};
            auto len = serial.read(buffer, sky::mcp_buffer_size);
            if (len == sky::mcp_buffer_size) {
                sky::mcp mcp = sky::mcp_make_from_buffer(buffer);
                print_mcp(mcp);
            }
        }
        
    }
}

auto cfg_listen() {
    set_mode(current_channel, hw::mode::rx);
    auto deltaTime = (unsigned long)random(250, 2000);
    auto startTime = millis();
    while (millis() - startTime < deltaTime){
        if (serial.available())
        {
            auto recived_message = serial.read();
            if (recived_message == MSG_REQUEST)
            {
                set_mode(current_channel, hw::mode::tx);
                serial.write(MSG_REQUEST_ACK);    //ACK to send
                serial.flush();
                set_mode(current_channel, hw::mode::rx);
                handle_packet();
            } else if (recived_message == MSG_REQUEST_ACK)
            {
                sky::mcp msg;
                msg.type = 0;
                for (int i = 0; i < 3; i++)
                {
                    msg.source[i] = id[i];
                    msg.destination[i] = 0;
                }
                for (int i = 0; i < 15; i++)
                {
                    msg.payload[i] = 0;
                }
                msg.crc = 0;

                sky::mcp_buffer_t msg_buffer;
                sky::mcp_make_buffer(msg_buffer, msg);         // Convert to buffer
                set_mode(current_channel, hw::mode::tx);
                serial.write(msg_buffer, sizeof(msg_buffer)); // Write buffer back
                serial.flush();
                set_mode(current_channel, hw::mode::rx);
            }
        }
    }
}

auto config_routine() -> state {
    while (cfg_count > 0) {
        Serial.printf("-------------- %d --------------\n", cfg_count);
        cfg_listen();        

        set_mode(current_channel, hw::mode::tx);
        serial.write(MSG_REQUEST);
        serial.flush();
        set_mode(current_channel, hw::mode::rx);
        while(temp > 0){
            cfg_listen();
            temp--;
        }
        temp = 5;

        switch (current_channel) {
            case hw::serial::ch0:
                current_channel = hw::serial::ch1;
                break;

            case hw::serial::ch1:
                    current_channel = hw::serial::ch2;
                    break;

            case hw::serial::ch2:
                current_channel = hw::serial::ch3;
                break;

            case hw::serial::ch3:
                current_channel = hw::serial::ch0;
                cfg_count--;
                break;
            
            default:
                Serial.println("Could not change channel");
                current_channel = hw::serial::ch0;
                break;
        }
    }
    return state::idle;
}

auto idle_routine() -> state {
    Serial.println("idle");
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

    serial.listen();

    
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
