/**
 * @file   multicom.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  Serial comunication over multiplexer.
 * @date   2022-12-07
 * 
 * @copyright Copyright (c) 2022
 */
#include "multicom.hpp"
#include "Arduino.h"

namespace ray {
multicom::multicom(int8_t rx_pin, int8_t tx_pin, uint32_t baud, control_register& control)
    : m_serial(rx_pin, tx_pin), m_baud(baud), m_control(control), m_in(), m_out() {
}
auto multicom::poll() -> void {
    if (m_current == state::receive) {
        m_control.set_com_channel(m_channel, 0);
        m_serial.begin(m_baud);
        m_serial.listen();
        m_current = state::wait;
    }

    if (m_current == state::wait) {
        if (m_current != m_previous) {
            m_start    = millis();
            m_interval = random(16, 66);
        } else if (millis() - m_start > m_interval) {
            m_current = state::done;
        }
    } 

    if (m_current == state::done) {
        if (m_serial.available()) {
            packet pkt{};
            pkt.channel = m_channel;
            pkt.size    = static_cast<uint8_t>(m_serial.available());
            m_serial.read(pkt.data, pkt.size);
            m_in[m_channel].enq(pkt);
        } else if (!m_out[m_channel].empty()) {
            // No data received, try to transmit data in current channel
            auto const pack = m_out[m_channel].deq();
            m_serial.flush();
            m_serial.stopListening();
            m_control.set_com_channel(m_channel, 1);
            m_serial.write(pack.data, pack.size);
            m_serial.flush();
        }

        // Switch to next channel and wait for data
        m_current = state::receive;
        m_channel = (m_channel + 1) % MAX_CHANNEL;
        m_serial.end();
    }

    m_previous = m_current;
}

auto multicom::write(packet pkt) -> void {
    if (pkt.channel >= MAX_CHANNEL) return;
    m_out[pkt.channel].enq(pkt);
}
auto multicom::read(uint8_t channel) -> packet {
    if (channel >= MAX_CHANNEL) return {};
    if (m_in[channel].empty()) return {};
    return m_in[channel].deq();
}
auto multicom::clear_buffer(uint8_t channel) noexcept -> void{
    if (channel >= MAX_CHANNEL) return;
    m_out[channel].clear();
    m_in[channel].clear();
}

} // namespace ray
