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
    m_state_timer.update(millis());
    m_control.set_com_channel(m_channel);

    if (m_serial.available() >= 4 && !m_serial.overflow()) {
        packet pkt{};
        pkt.size = m_serial.available();
        m_serial.read(pkt.data, pkt.size);
        m_in[m_channel].enq(std::move(pkt));
    }

    if (m_out[m_channel].size() > 0) {
        auto const pkt = m_out[m_channel].deq();
        m_serial.write(pkt.data, pkt.size);
    }

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
