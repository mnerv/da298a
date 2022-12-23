/**
 * @file   porter.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  Serial comunication over multiplexer.
 * @date   2022-12-07
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef SUNLIGHT_PORTER_HPP
#define SUNLIGHT_PORTER_HPP

#include "SoftwareSerial.h"

#include "sky.hpp"
#include "control_register.hpp"

namespace ray {
constexpr std::size_t MAX_QUEUE   = 16;
constexpr std::size_t MAX_CHANNEL = 4;

constexpr size_t PREAMBLE_SIZE = 2;
constexpr uint8_t PREAMBLE_PATTERN = 0b1010'1010;
constexpr uint8_t SFD_PATTERN      = 0b1010'1010;
constexpr size_t  PREAMBLE_SFD_SIZE = PREAMBLE_SIZE + 1;

constexpr size_t PACKET_SIZE = 32;

// Takes into account preamble and SFD.
constexpr size_t PACKET_DATA_SIZE = PACKET_SIZE - PREAMBLE_SFD_SIZE;

struct packet {
    uint8_t channel = 0;
    uint8_t size    = 0;
    uint8_t data[PACKET_DATA_SIZE]{};
};

class porter {
public:
    porter(int8_t rx_pin, int8_t tx_pin, uint32_t baud, control_register& control);

    auto begin() -> void;
    auto poll() -> void;
    auto channel() const -> uint8_t { return m_channel; }
    auto is_wait() const -> bool { return m_current == state::wait; }
    auto is_receive() const -> bool { return m_current == state::receive; }

    auto write(packet pkt) noexcept -> void;
    auto read(uint8_t channel) noexcept -> packet;

private:
    SoftwareSerial m_serial;
    uint32_t m_baud;
    control_register& m_control;
    sky::queue<packet, MAX_QUEUE> m_in[MAX_CHANNEL];
    sky::queue<packet, MAX_QUEUE> m_out[MAX_CHANNEL];

    sky::queue<uint8_t, 256> m_in_bytes[MAX_CHANNEL];

    enum class state {
        wait,
        receive,
        send,
        wait_for_listen,
    };
    state m_current{state::wait};
    state m_next{state::wait};

    uint8_t  m_channel  = 0;
    uint32_t m_start    = 0;
    uint32_t m_interval = 0;
    uint32_t m_current_time = 0;
};
} // namespace ray

#endif  // !SUNLIGHT_PORTER_HPP
