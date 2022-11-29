/**
 * @file   mode.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  Mode types.
 * @date   2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef SUNLIGHT_MODE_HPP
#define SUNLIGHT_MODE_HPP

#include <type_traits>
#include <stdint.h>

namespace hw {
enum class serial : uint8_t {
    ch0 = 0b0000'0000,
    ch1 = 0b0000'0001,
    ch2 = 0b0000'0010,
    ch3 = 0b0000'0011,
};

enum class mode : uint8_t {
    tx = 0b0000'0100,
    rx = 0b0000'0000,
};

inline constexpr auto operator|(serial const& lhs, mode const& rhs) -> uint8_t {
    using T = std::underlying_type<serial>::type;
    using U = std::underlying_type<mode>::type;
    static_assert(std::is_same<T, U>::value, "Underlying types must be the same");
    return static_cast<T>(lhs) | static_cast<T>(rhs);
}
inline constexpr auto operator|(mode const& lhs, serial const& rhs) -> uint8_t{
    return rhs | lhs;
}

inline constexpr auto operator&(serial const& lhs, mode const& rhs) -> uint8_t {
    using T = std::underlying_type<serial>::type;
    using U = std::underlying_type<mode>::type;
    static_assert(std::is_same<T, U>::value, "Underlying types must be the same");
    return static_cast<T>(lhs) & static_cast<T>(rhs);
}
inline constexpr auto operator&(mode const& lhs, serial const& rhs) -> uint8_t{
    return rhs & lhs;
}

} // namespace hw

#endif  // !SUNLIGHT_MODE_HPP
