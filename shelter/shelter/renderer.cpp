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

#include "glm/gtc/matrix_transform.hpp"

namespace shelter {

auto make_renderer(graphics_context_ref_t context) -> renderer_ref_t {
    if (context->window()->renderer() != nullptr) return context->window()->renderer();
    context->window()->set_renderer(make_ref<renderer>(context));
    return context->window()->renderer();
}

renderer::renderer(graphics_context_ref_t context) : m_context(std::move(context)) {
    setup_2d();

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
auto renderer::begin(camera_ref_t const& camera) -> void {
    m_camera = camera;
}
auto renderer::submit(shader_ref_t const& shader, vertex_buffer_ref_t const& vb, index_buffer_ref_t const& ib, glm::mat4 const& model) -> void {
    shader->bind();
    shader->upload("u_model", model);
    shader->upload("u_view", m_camera->view());
    shader->upload("u_projection", m_camera->projection());
    vb->bind();
    ib->bind();
    glDrawElements(GL_TRIANGLES, ib->size(), ib->type(), nullptr);
}
auto renderer::quad(glm::vec2 const& position, glm::vec2 const& size, glm::vec4 const& color) -> void {
    m_2d.shader->upload("u_color", color);
    submit(m_2d.shader, m_2d.vertex_buffer, m_2d.index_buffer,
        glm::scale(glm::translate(glm::mat4{1.0f}, glm::vec3{position, 0.0f}), glm::vec3{size, 1.0f}));
}
auto renderer::end() -> void {
    // TODO: clean up
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
auto renderer::setup_2d() -> void {
    struct vertex {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 uv;
    };

    static vertex vertices[] {
        {{-0.5f,  0.5f,  0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    };

    static std::uint32_t indices[] {
        0, 1, 2,
        0, 2, 3
    };

    m_2d.shader = shelter::make_shader(m_context);
    m_2d.index_buffer = shelter::make_index_buffer(m_context, indices, sizeof(indices),
        static_cast<std::uint32_t>(shelter::length_of(indices)));
    m_2d.vertex_buffer = shelter::make_vertex_buffer(m_context, vertices, sizeof(vertices), {
        {shelter::data_type::vec3, "a_position"},
        {shelter::data_type::vec4, "a_color"},
        {shelter::data_type::vec2, "a_uv"},
    });
}
} // namespace shelter
