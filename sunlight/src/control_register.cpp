#include "control_register.hpp"

namespace ray {
auto control_register::set_com_channel(uint8_t ch) -> void {
    sky::set_bit_level<uint8_t>(m_buffer, 0b0000'0011, ch);
    m_register_fn(m_buffer);
}
auto control_register::set_config_channel(uint8_t ch) -> void {
    sky::set_bit_level<uint8_t>(m_buffer, 0b0001'1100, ch << 2);
    m_register_fn(m_buffer);
}
} // namespace ray
