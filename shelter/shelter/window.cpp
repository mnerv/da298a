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
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable Multi-Viewport / Platform Windows

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
}
window::~window() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
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
auto window::poll() -> void {
    glfwPollEvents();
    glfwGetWindowSize(m_window, &m_data.width, &m_data.height);
}
auto window::swap() -> void {
    glfwSwapBuffers(m_window);
}

auto window::begin_imgui() -> void {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
auto window::end_imgui() -> void {
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        auto backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

} // namespace shelter
