/**
 * @file   multicom.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  Serial comunication over multiplexer.
 * @date   2022-12-07
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef SUNLIGHT_MULTICOM_HPP
#define SUNLIGHT_MULTICOM_HPP

#include "SoftwareSerial.h"

#include "sky.hpp"
#include "control_register.hpp"

namespace ray {
constexpr std::size_t MAX_QUEUE   = 16;
constexpr std::size_t MAX_CHANNEL = 4;

struct packet {
    uint8_t channel = 0;
    uint8_t size    = 0;
    uint8_t data[32]{};
};

class multicom {
public:
    multicom(int8_t rx_pin, int8_t tx_pin, uint32_t baud, control_register& control);

    auto poll() -> void;

    auto write(packet pkt) noexcept -> void;
    auto read(uint8_t channel) noexcept -> packet;

private:
    SoftwareSerial m_serial;
    uint32_t m_baud;
    control_register& m_control;
    sky::queue<packet, MAX_QUEUE> m_in[MAX_CHANNEL];
    sky::queue<packet, MAX_QUEUE> m_out[MAX_CHANNEL];

    enum class state {
        receive,
        wait,
        done,
    };
    state m_current{state::receive};
    state m_previous{state::receive};

    uint8_t  m_channel  = 0;
    uint32_t m_start    = 0;
    uint32_t m_interval = 0;
};
} // namespace ray

#endif  // !SUNLIGHT_MULTICOM_HPP
