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

auto multicom::begin() -> void {
    m_serial.begin(m_baud);
    m_serial.listen();
}
auto multicom::poll() -> void {
    m_state_timer.update(millis());
    m_control.set_com_channel(m_channel, 0);

    switch (m_current) {
    case state::wait: {
        if (m_serial.available())
            m_next = state::receive;
        break;
    }
    case state::receive: {
        m_next = state::wait;
        while (m_serial.available()){
            char c = m_serial.read();
            if (c == 'R') m_next = state::wait_for_listen;
        }
        break;
    }
    case state::send: {
        m_control.set_com_channel(m_channel, 1);

        [[maybe_unused]]auto pkt = m_out[m_channel].deq();
        uint8_t buffer[] = "NERV";
        m_serial.write(buffer, sky::length_of(buffer));
        m_serial.flush();

        m_next = state::wait;
        break;
    }
    default: break;
    }

    m_current = m_next;
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
} // namespace ray
