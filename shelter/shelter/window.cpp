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
    m_data.buffer_width  = props.width;
    m_data.buffer_height = props.height;
    m_data.xpos   = 0;
    m_data.ypos   = 0;
    m_data.xscale = 1.0f;
    m_data.yscale = 1.0f;

    if (!glfwInit()) throw std::runtime_error("shelter::window: error: Failed to initialize GLFW!");
    setup_opengl();
    m_window = glfwCreateWindow(m_data.width, m_data.height, m_data.title.c_str(), nullptr, nullptr);
    if (!m_window) throw std::runtime_error("shelter::window: error: Failed to create GLFW window instance!");

    glfwMakeContextCurrent(m_window);
    if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress)))
        throw std::runtime_error("shelter::window: error: Failed to load glad!");

    setup_events();
}
window::~window() {
    glfwTerminate();
}

auto window::title() const -> std::string const& { return m_data.title; }
auto window::width() const -> std::int32_t { return m_data.width; }
auto window::height() const -> std::int32_t { return m_data.height; }
auto window::buffer_width()  const -> std::int32_t { return m_data.buffer_width; }
auto window::buffer_height() const -> std::int32_t { return m_data.buffer_height; }
auto window::xpos() const -> std::int32_t { return m_data.xpos; }
auto window::ypos() const -> std::int32_t { return m_data.ypos; }
auto window::native() const -> GLFWwindow* { return m_window; }
auto window::context() const -> graphics_context_ref_t const& { return m_context; }
auto window::renderer() const -> renderer_ref_t const& { return m_renderer; }
auto window::time() const -> double { return glfwGetTime(); }

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
    glm::dvec2 pos{};
    glfwGetCursorPos(m_window, &pos.x, &pos.y);
    return pos;
}
auto window::key(std::int32_t const& key) const -> std::int32_t {
    return glfwGetKey(m_window, key);
}
auto window::mouse(std::int32_t const& button) const -> std::int32_t {
    return glfwGetMouseButton(m_window, button);
}
auto window::poll() -> void { glfwPollEvents(); }

auto window::setup_events() -> void {
    glfwSetWindowUserPointer(m_window, &m_data);
    glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, std::int32_t width, std::int32_t height) {
        auto data = window::user_ptr(window);
        data->width  = width;
        data->height = height;
    });
    glfwSetWindowPosCallback(m_window, [](GLFWwindow* window, std::int32_t xpos, std::int32_t ypos) {
        auto data = window::user_ptr(window);
        data->xpos = xpos;
        data->ypos = ypos;
    });
    //glfwSetWindowFocusCallback(m_window, [](GLFWwindow* window, std::int32_t focused) {
    //    auto data = window::user_ptr(window);
    //});
    //glfwSetWindowIconifyCallback(m_window, [](GLFWwindow* window, std::int32_t iconified) {
    //    auto data = window::user_ptr(window);
    //});
    //glfwSetWindowMaximizeCallback(m_window, [](GLFWwindow* window, std::int32_t maximized) {
    //    auto data = window::user_ptr(window);
    //});
    glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, std::int32_t width, std::int32_t height) {
        auto data = window::user_ptr(window);
        data->buffer_width  = width;
        data->buffer_height = height;
    });
    glfwSetWindowContentScaleCallback(m_window,
    [](GLFWwindow* window, float xscale, float yscale){
        auto data = window::user_ptr(window);
        data->xscale = xscale;
        data->yscale = yscale;
    });
    //glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xpos, double ypos) {
    //    auto data = window::user_ptr(window);
    //});
    //glfwSetCursorEnterCallback(m_window, [](GLFWwindow* window, std::int32_t entered) {
    //    auto data = window::user_ptr(window);
    //});
    //glfwSetMouseButtonCallback(m_window,
    //[](GLFWwindow* window, std::int32_t button, std::int32_t action, std::int32_t mods) {
    //    auto data = window::user_ptr(window);
    //});
    //glfwSetScrollCallback(m_window, 
    //    [](GLFWwindow* window, double xoffset, double yoffset) {
    //    auto data = window::user_ptr(window);
    //});
    //glfwSetKeyCallback(m_window,
    //[](GLFWwindow* window, std::int32_t key, std::int32_t code, std::int32_t action, std::int32_t mods) {
    //    auto data = window::user_ptr(window);
    //});
    //glfwSetCharCallback(m_window,
    //[](GLFWwindow* window, std::uint32_t codepoint) {
    //    auto data = window::user_ptr(window);
    //});
    //glfwSetDropCallback(m_window, [](GLFWwindow* window, std::int32_t count, char const** paths) {
    //    auto data = window::user_ptr(window);
    //});
    glfwGetFramebufferSize(m_window, &m_data.buffer_width, &m_data.buffer_height);
    glfwGetWindowPos(m_window, &m_data.xpos, &m_data.ypos);
}

auto window::user_ptr(GLFWwindow* window) -> window::data* {
    return static_cast<window::data*>(glfwGetWindowUserPointer(window));
}
} // namespace shelter
