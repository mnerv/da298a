#include "control_register.hpp"

namespace ray {
auto control_register::set_com_channel(uint8_t ch, uint8_t mode) -> void {
    sky::set_bit_level<uint8_t>(m_buffer, 0b0000'0111, ch | mode);
    m_register_fn(m_buffer);
}
} // namespace ray
