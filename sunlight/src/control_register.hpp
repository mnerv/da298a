/**
 * @file   control_register.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  Control register storage for shift register.
 * @date   2022-12-07
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef SUNLIGHT_CONTROL_REGISTER_HPP
#define SUNLIGHT_CONTROL_REGISTER_HPP

#include "sky.hpp"
#include <functional>

namespace ray {
constexpr uint8_t COM_RX_MODE = 0b0000'0000;  // MAX485 - Receive mode
constexpr uint8_t COM_TX_MODE = 0b0000'0100;  // MAX485 - Transmit mode

class control_register {
public:
    using register_fn_t = std::function<void(uint8_t)>;

public:
    auto set_register(register_fn_t fn) -> void { m_register_fn = std::move(fn); }

    auto set_com_channel(uint8_t ch, uint8_t mode) -> void;

private:
    uint8_t m_buffer    = 0;
    uint8_t m_clock_pin = 0;
    uint8_t m_data_pin  = 0;
    uint8_t m_latch_pin = 0;
    register_fn_t m_register_fn{};
};
} // namespace ray

#endif  // !SUNLIGHT_CONTROL_REGISTER_HPP
