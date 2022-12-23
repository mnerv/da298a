/**
 * @file   porter.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  Serial comunication over multiplexer.
 * @date   2022-12-07
 * 
 * @copyright Copyright (c) 2022
 */
#include "porter.hpp"
#include "Arduino.h"

namespace ray {
porter::porter(int8_t rx_pin, int8_t tx_pin, uint32_t baud, control_register& control)
    : m_serial(rx_pin, tx_pin), m_baud(baud), m_control(control), m_in(), m_out() {
}

auto porter::begin() -> void {
    m_serial.begin(m_baud);
    m_serial.listen();
}
auto porter::poll() -> void {
    m_control.set_com_channel(m_channel);

    if (m_serial.available()) {
        m_in_bytes[m_channel].enq(uint8_t(m_serial.read()));

        auto const check_preamble = [](uint8_t const& byte) {
            return byte == PREAMBLE_PATTERN;
        };
        auto const check_sfd = [](uint8_t const& byte) {
            return byte == SFD_PATTERN;
        };

        // Check preamble
        auto is_preamble_ok = true;
        for (size_t i = 0; i < PREAMBLE_SIZE; ++i) {
            if (check_preamble(m_in_bytes[m_channel].peek(i))) {
                is_preamble_ok = true;
            } else {
                for (size_t j  = 0; j <= i; ++j) {
                    m_in_bytes[m_channel].pop_front();
                }
                is_preamble_ok = false;
                break;
            }
        }

        if (is_preamble_ok) {
            // Check start frame delimiter
            auto is_sfd_ok = check_sfd(m_in_bytes[m_channel].peek(PREAMBLE_SIZE));

            if (is_sfd_ok && m_in_bytes[m_channel].size() >= PACKET_SIZE) {
                uint8_t buffer[PACKET_SIZE] = {0};
                auto size = m_in_bytes[m_channel].size();
                for (size_t i = 0; i < size; i++)
                {
                    buffer[i] = m_in_bytes[m_channel].deq();
                }

                packet pkt{};
                pkt.channel = m_channel;
                pkt.size    = PACKET_DATA_SIZE;
                memcpy(pkt.data, buffer + PREAMBLE_SFD_SIZE, PACKET_DATA_SIZE);
                m_in[m_channel].enq(pkt);
            }
        }
    }

    if (!m_out[m_channel].empty()) {
        // Packet size is 32 bytes
        // Preamble is 2 bytes (0b1010'1010) and SFD is 1 byte (0b1010'1011)
        auto const pkt = m_out[m_channel].deq();
        uint8_t buffer[PACKET_SIZE] = {0};
        buffer[0] = PREAMBLE_PATTERN;
        buffer[1] = PREAMBLE_PATTERN;
        buffer[2] = SFD_PATTERN;
        memcpy(buffer + PREAMBLE_SFD_SIZE, pkt.data, pkt.size);
        m_serial.write(buffer, PACKET_SIZE);
        m_serial.flush();
    }

    // TODO: When to switch channel

    m_current = m_next;
}

auto porter::write(packet pkt) -> void {
    if (pkt.channel >= MAX_CHANNEL) return;
    m_out[pkt.channel].enq(pkt);
}
auto porter::read(uint8_t channel) -> packet {
    if (channel >= MAX_CHANNEL) return {};
    if (m_in[channel].empty()) return {};
    return m_in[channel].deq();
}
} // namespace ray
