/*
 * @file   graphics_context.cpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  graphics_context abstraction
 * @date   2022-11-08
 *
 * @copyright Copyright (c) 2022
 */
#include "graphics_context.hpp"
#include "fmt/format.h"

namespace shelter {
static auto info_opengl() -> void {
    fmt::print("OpenGL Info:\n");
    fmt::print("    Vendor:   {:s}\n", (char const*)glGetString(GL_VENDOR));
    fmt::print("    Renderer: {:s}\n", (char const*)glGetString(GL_RENDERER));
    fmt::print("    Version:  {:s}\n", (char const*)glGetString(GL_VERSION));
    fmt::print("    Shader:   {:s}\n", (char const*)glGetString(GL_SHADING_LANGUAGE_VERSION));
}

auto make_graphics_context(window_ref_t window) -> graphics_context_ref_t {
    if (window->context() != nullptr) return window->context();
    window->set_context(make_ref<graphics_context>(window));
    return window->context();
}

graphics_context::graphics_context(window_ref_t window) : m_window(std::move(window)) {
    info_opengl();
}
graphics_context::~graphics_context() = default;

auto graphics_context::viewport(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) const -> void {
    glViewport(x, y, width, height);
}
auto graphics_context::set_clear_color(glm::vec4 const& color) const -> void {
    glClearColor(color.r, color.g, color.b, color.a);
}
auto graphics_context::clear(std::uint32_t const& mask) const -> void { glClear(mask); }
auto graphics_context::swap() const -> void { glfwSwapBuffers(m_window->native()); }

} // namespace shelter

