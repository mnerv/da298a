/**
 * @file   config_status.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  Node configuration and status.
 * @date 2022-12-20
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef SUNLIGHT_CONFIG_STATUS_HPP
#define SUNLIGHT_CONFIG_STATUS_HPP
#include "control_register.hpp"

namespace ray {
class config_status {
public:
    config_status(uint8_t pin, control_register& control);

    auto is_reset() const -> bool;
    auto is_fire() const -> bool;
    auto is_exit() const -> bool;

private:
    uint8_t m_pin;
    control_register& m_control;
};



} // namespace ray


#endif  // !SUNLIGHT_CONFIG_STATUS_HPP
