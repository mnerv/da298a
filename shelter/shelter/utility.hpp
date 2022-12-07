/**
 * @file   utility.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  Utility functions for shelter
 * @date   2022-11-11
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef SHELTER_UTILITY_HPP
#define SHELTER_UTILITY_HPP

#include <memory>

namespace shelter {
template <typename T>
using ref = std::shared_ptr<T>;
template <typename T>
using local = std::unique_ptr<T>;

template <typename T>
using enable_ref = std::enable_shared_from_this<T>;

template <typename T, typename... Args>
constexpr auto make_ref(Args&&... args) -> ref<T> {
    return std::make_shared<T>(std::forward<Args>(args)...);
}
template <typename T, typename... Args>
constexpr auto make_local(Args&&... args) -> local<T> {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T, std::size_t N>
constexpr auto length_of(T (&)[N]) -> std::size_t {
    return N;
}
}

#endif  // SHELTER_UTILITY_HPP
