/*
 * @file   graphics_context.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  graphics_context abstraction
 * @date   2022-11-08
 *
 * @copyright Copyright (c) 2022
 */
#ifndef SHELTER_GRAPHICS_CONTEXT_HPP
#define SHELTER_GRAPHICS_CONTEXT_HPP

#include "common.hpp"
#include "window.hpp"
#include "glad/glad.h"
#include "glm/vec4.hpp"

namespace shelter {
using graphics_context_ref_t = ref<class graphics_context>;
auto make_graphics_context(window_ref_t window) -> graphics_context_ref_t;

class graphics_context {
public:
    graphics_context(window_ref_t window);
    ~graphics_context();

    auto viewport(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) const -> void;
    auto set_clear_color(glm::vec4 const& color = {1.0f, 0.0f, 1.0f, 1.0f}) const -> void;
    auto clear(std::uint32_t const& mask = GL_COLOR_BUFFER_BIT) const -> void;
    auto swap() const -> void;

    auto window() const -> window_ref_t { return m_window; }

private:
    window_ref_t m_window;
};
} // namespace shelter

#endif  // SHELTER_GRAPHICS_CONTEXT_HPP
