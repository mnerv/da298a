/**
 * @file   common.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  Common includes
 * @date   2022-11-08
 *
 * @copyright Copyright (c) 2022
 */
#ifndef SHELTER_COMMON_HPP
#define SHELTER_COMMON_HPP

#include <memory>

namespace shelter {
template <typename T>
using ref = std::shared_ptr<T>;
template <typename T>
using local = std::unique_ptr<T>;

template <typename T, typename... Args>
constexpr auto make_ref(Args&&... args) -> ref<T> {
    return std::make_shared<T>(std::forward<Args>(args)...);
}
template <typename T, typename... Args>
constexpr auto make_local(Args&&... args) -> local<T> {
    return std::make_unique<T>(std::forward<Args>(args)...);
}
} // namespace shelter

#endif  // SHELTER_COMMON_HPP
