/**
 * @file   window.cpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  window abstraction implementation
 * @date   2022-11-08
 *
 * @copyright Copyright (c) 2022
 */
#include <stdexcept>
#include "window.hpp"

#include "glad/glad.h"

namespace shelter {
static auto setup_opengl() -> void {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

auto make_window(window_props const& props) -> window_ref_t {
    return make_ref<window>(props);
}

window::window(window_props const& props) {
    m_data.title  = props.title;
    m_data.width  = props.width;
    m_data.height = props.height;

    if (!glfwInit()) throw std::runtime_error("shelter::window: error: Failed to initialize GLFW!");
    setup_opengl();
    m_window = glfwCreateWindow(m_data.width, m_data.height, m_data.title.c_str(), nullptr, nullptr);
    if (!m_window) throw std::runtime_error("shelter::window: error: Failed to create GLFW window instance!");

    glfwMakeContextCurrent(m_window);
    if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress)))
        throw std::runtime_error("shelter::window: error: Failed to load glad!");
}
window::~window() {
    glfwTerminate();
}

auto window::context() const -> graphics_context_ref_t const& {
    return m_context;
}
auto window::renderer() const -> renderer_ref_t const& {
    return m_renderer;
}
auto window::time() const -> double {
    return glfwGetTime();
}

auto window::set_title(std::string const& title) -> void {
    m_data.title = title;
    glfwSetWindowTitle(m_window, m_data.title.c_str());
}
auto window::set_width(std::int32_t const& width) -> void {
    m_data.width = width;
    glfwSetWindowSize(m_window, m_data.width, m_data.height);
}
auto window::set_height(std::int32_t const& height) -> void {
    m_data.height = height;
    glfwSetWindowSize(m_window, m_data.width, m_data.height);
}
auto window::set_context(graphics_context_ref_t context) -> void {
    m_context = std::move(context);
}
auto window::set_renderer(renderer_ref_t renderer) -> void {
    m_renderer = std::move(renderer);
}

auto window::shouldclose() const -> bool {
    return glfwWindowShouldClose(m_window);
}
auto window::mouse_pos() const -> glm::dvec2 {
    glm::dvec2 pos;
    glfwGetCursorPos(m_window, &pos.x, &pos.y);
    return pos;
}
auto window::key(std::int32_t const& key) const -> std::int32_t {
    return glfwGetKey(m_window, key);
}
auto window::mouse(std::int32_t const& button) const -> std::int32_t {
    return glfwGetMouseButton(m_window, button);
}
auto window::poll() -> void {
    glfwPollEvents();
    glfwGetWindowSize(m_window, &m_data.width, &m_data.height);
    std::int32_t buffer_width, buffer_height;
    glfwGetFramebufferSize(m_window, &buffer_width, &buffer_height);
    if (buffer_width >= 0 && buffer_height >= 0) {
        m_data.buffer_width  = static_cast<std::uint32_t>(buffer_width);
        m_data.buffer_height = static_cast<std::uint32_t>(buffer_height);
    }
}

} // namespace shelter
