/**
 * @file    utility.hpp
 * @author  Pratchaya Khansomboon (me@mononerv.dev)
 * @brief   sky utility functions
 * @version 0.1
 * @date    2022-12-23
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef SKY_UTILITY_HPP
#define SKY_UTILITY_HPP

#include <cstddef>
#include <type_traits>

namespace sky {
template <typename T, std::size_t N>
inline constexpr auto length_of(T (&)[N]) -> std::size_t {
    return N;
}

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
inline constexpr auto set_bit(T& reg, std::size_t const& position) noexcept -> void {
    reg |= 1 << position;
}

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
inline constexpr auto clear_bit(T& reg, std::size_t const& position) noexcept -> void {
    reg &= ~(1 << position);
}

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
inline constexpr auto set_bit_level(T& reg, T const& mask, T const& data) noexcept -> void {
    reg = (reg & ~mask) | (data & mask);
}

template <std::uint8_t POLY = 0x07>
inline constexpr auto crc_8(std::uint8_t const* buffer, std::size_t size) -> std::uint8_t {
    constexpr std::uint8_t poly = POLY;
    std::uint8_t reg = 0x00;
    for (std::size_t i = 0; i < size; ++i) {
        reg = reg ^ buffer[i];
        for (std::uint8_t j = 0; j < 8; ++j) {
            if (reg & 0x80) reg = (reg << 1) ^ poly;
            else reg = (reg << 1);
        }
    }
    return reg;
}
} // namespace sky

#endif  // !SKY_UTILITY_HPP
