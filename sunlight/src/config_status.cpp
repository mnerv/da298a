#include "config_status.hpp"
#include "Arduino.h"

namespace ray {
config_status::config_status(uint8_t pin, control_register& control) : m_pin(pin), m_control(control) {}

auto config_status::is_reset() const -> bool {
    m_control.set_config_channel(0);
    return read_pin();
}
auto config_status::is_fire() const -> bool {
    m_control.set_config_channel(1);
    return read_pin();
}
auto config_status::is_exit() const -> bool {
    m_control.set_config_channel(2);
    return read_pin();
}

auto config_status::read_pin() const -> bool {
    return !(analogRead(m_pin) > 512);
}
}
