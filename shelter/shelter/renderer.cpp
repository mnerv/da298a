/*
 * @file   renderer.cpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  renderer abstraction
 * @date   2022-11-08
 *
 * @copyright Copyright (c) 2022
 */
#include "renderer.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace shelter {
auto make_renderer(graphics_context_ref_t context) -> renderer_ref_t {
    if (context->window()->renderer() != nullptr) return context->window()->renderer();
    context->window()->set_renderer(make_ref<renderer>(context));
    return context->window()->renderer();
}

renderer::renderer(graphics_context_ref_t context) : m_context(std::move(context)) {
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

    ImGui_ImplGlfw_InitForOpenGL(m_context->window()->native(), true);
    ImGui_ImplOpenGL3_Init("#version 410");
}
renderer::~renderer() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

auto renderer::begin_imgui() const -> void {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
auto renderer::end_imgui() const -> void {
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
