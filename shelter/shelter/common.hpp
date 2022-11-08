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

template <typename T, std::size_t N>
constexpr auto length_of(T (&)[N]) -> std::size_t {
    return N;
}

// Foward declaration
using window_ref_t           = ref<class window>;
using graphics_context_ref_t = ref<class graphics_context>;
using shader_ref_t           = ref<class shader>;
using vertex_buffer_ref_t    = ref<class vertex_buffer>;
using index_buffer_ref_t     = ref<class index_buffer>;
using renderer_ref_t         = ref<class renderer>;
using camera_ref_t           = ref<class camera>;

} // namespace shelter

#endif  // SHELTER_COMMON_HPP
