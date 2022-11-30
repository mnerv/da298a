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

template <typename T, size_t SIZE>
class queue {
public:
    auto enq(T const& data) -> void {
        if (m_size == SIZE) inc(m_head, SIZE);
        m_buffer[m_tail] = data;
        inc(m_tail, SIZE);
        ++m_size;
    }

    auto deq() -> T {
        if (m_head == m_tail) return m_buffer[m_head];
        auto const& value = m_buffer[m_head];
        inc(m_head, SIZE);
        --m_size;
        return value;
    }

    auto empty() const -> bool {
        return m_size == 0;
    }

    auto peek(size_t i) const -> T {
        return m_buffer[i];
    }

    auto size() const -> size_t {
        return m_size;
    }
    auto capacity() const -> size_t {
        return SIZE;
    }

private:
    static auto inc(size_t& value, size_t const& size) -> void {
        value = (value + 1) % size;
    }
    auto next(size_t const& value) const -> size_t {
        return (value + 1) % SIZE;
    }

private:
    T m_buffer[SIZE]{};
    size_t m_head = 0;
    size_t m_tail = 0;
    size_t m_size = 0;
};

struct event {
    hw::serial channel = hw::serial::ch0;
    size_t     size{};
    uint8_t    data[32];
};

queue<event, 16> event_queue{};

auto set_mode(hw::serial const& ch, hw::mode const& mode) -> void{
    digitalWrite(SR_LATCH_PIN, LOW);
    shiftOut(SR_DATA_PIN, SR_CLK_PIN, MSBFIRST, ch | mode);
    digitalWrite(SR_LATCH_PIN, HIGH);
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

auto on_message(event const& e) -> void {
    sky::mcp_buffer_t buffer{};
    memcpy(buffer, e.data, sky::mcp_buffer_size);

    (void)buffer;
    switch (e.channel) {
    case hw::serial::ch0:
        Serial.print("ch0: ");
    break;
    case hw::serial::ch1:
        Serial.print("ch1: ");
    break;
    case hw::serial::ch2:
        Serial.print("ch2: ");
    break;
    case hw::serial::ch3:
        Serial.print("ch3: ");
    break;
    }

    if (e.size == sky::mcp_buffer_size) {
        sky::mcp mcp = sky::mcp_make_from_buffer(buffer);
        print_mcp(mcp);
    } else {
        Serial.println("invalid data");
    }
}

enum class rxtx_state : uint8_t {
    rx,
    wait,
    done,
    tx,
};

rxtx_state previous_rxtx = rxtx_state::rx;
rxtx_state current_rxtx = rxtx_state::rx;
uint64_t rxtx_start_time = 0;
uint64_t rxtx_wait_time = 0;

uint64_t start_time = 0;
uint64_t wait_time  = 500;

hw::serial channels[4] {
    hw::serial::ch0,
    hw::serial::ch1,
    hw::serial::ch2,
    hw::serial::ch3,
};

uint8_t current_channel = 0;

auto rx_tx() -> void {
    if (current_rxtx == rxtx_state::rx) {
        current_channel = (current_channel + 1) % 4;
        set_mode(channels[current_channel], hw::mode::rx);
        serial.listen();
        current_rxtx = rxtx_state::wait;
    }

    if (current_rxtx == rxtx_state::wait) {
        pixel.setPixelColor(0, 0xFF0000);
        pixel.show();
        if (current_rxtx != previous_rxtx) {
            rxtx_start_time = millis();
            rxtx_wait_time  = random(250, 1250);
            previous_rxtx   = current_rxtx;
        } else {
            if (millis() - rxtx_start_time > rxtx_wait_time)
                current_rxtx = rxtx_state::done;
        }
    }

    if (current_rxtx == rxtx_state::done) {
        pixel.setPixelColor(0, 0x00FF00);
        pixel.show();
        if (serial.available()) {
            event ev;
            ev.channel = channels[current_channel];
            ev.size = serial.available();
            serial.read(ev.data, ev.size);
            on_message(ev);
        } else {
            current_rxtx = rxtx_state::tx;
        }
        previous_rxtx = current_rxtx;
    }

    if (current_rxtx == rxtx_state::tx) {
        serial.flush();
        serial.stopListening();

        if (event_queue.empty()) {
            current_rxtx = rxtx_state::rx;
        } else {
            pixel.setPixelColor(0, 0x0000FF);
            pixel.show();
            auto const& msg = event_queue.deq();
            set_mode(msg.channel, hw::mode::tx);
            serial.write(msg.data, msg.size);
            serial.flush();
        }
    }
}

auto send_message(hw::serial const& ch, sky::mcp const& mcp) -> void {
    event msg;
    msg.channel = ch;
    msg.size = sky::mcp_buffer_size;
    sky::mcp_buffer_t buffer;
    sky::mcp_make_buffer(buffer, mcp);
    memcpy(msg.data, buffer, sky::mcp_buffer_size);
    event_queue.enq(msg);
}

void setup() {
    Serial.begin(HARDWARE_BAUD);
    serial.begin(SOFTWARE_BAUD);

    pinMode(SR_CLK_PIN,  OUTPUT);
    pinMode(SR_DATA_PIN, OUTPUT);
    pinMode(SR_LATCH_PIN, OUTPUT);

    pixel.begin();
}

void loop() {
    rx_tx();

    if (millis() - start_time > wait_time) {
        start_time = millis();
        sky::mcp_buffer_t buffer{};
        sky::mcp mcp{ 0, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 };
        sky::mcp_u32_to_address(mcp.source, ESP.getChipId());
        sky::mcp_make_buffer(buffer, mcp);
        send_message(hw::serial::ch0, mcp);
        send_message(hw::serial::ch1, mcp);
        send_message(hw::serial::ch2, mcp);
        send_message(hw::serial::ch3, mcp);
    }
}
