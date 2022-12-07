/**
 * @file   sky.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @author Christian Heisterkamp
 * @author Petter Rignell
 * @brief  sky framework
 * @date   2022-10-19
 *
 * @copyright Copyright (c) 2022
 */
#ifndef SKY_SKY_HPP
#define SKY_SKY_HPP

#include <cstddef>
#include <type_traits>

#include "mcp.hpp"
#include "topo.hpp"
#include "queue.hpp"

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
}

#endif  // !SKY_SKY_HPP
