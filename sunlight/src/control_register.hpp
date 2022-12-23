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
class control_register {
public:
    using register_fn_t = std::function<void(uint8_t)>;

public:
    auto set_register(register_fn_t fn) -> void { m_register_fn = std::move(fn); }

    /**
     * @brief Set the com channel for shift register.
     * 
     * @param ch 0-3.
     * @param mode TX or RX mode. 1 for TX, 0 for RX.
     */
    auto set_com_channel(uint8_t ch, uint8_t mode) -> void;

    /**
     * @brief Set the config channel for shift register.
     * @param ch 0-7.
     */
    auto set_config_channel(uint8_t ch) -> void;

private:
    uint8_t m_buffer = 0;
    register_fn_t m_register_fn{};
};
} // namespace ray

#endif  // !SUNLIGHT_CONTROL_REGISTER_HPP
