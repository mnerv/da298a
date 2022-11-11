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

static constexpr auto DEFAULT_VERTEX_SHADER = R"(#version 410 core
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_uv;

out vec4 io_color;
out vec2 io_uv;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
    io_color = a_color;
    io_uv    = a_uv;

    gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0f);
}
)";
static constexpr auto DEFAULT_FRAGMENT_SHADER = R"(#version 410 core
layout(location = 0) out vec4 color;

in vec4 io_color;
in vec2 io_uv;

uniform vec4 u_color;

void main() {
    color = u_color;
}
)";
static constexpr auto CIRCLE_FRAGMENT_SHADER = R"(#version 410 core
layout(location = 0) out vec4 color;
in vec4 io_color;
in vec2 io_uv;

uniform vec4 u_color;

void main() {
    if (length(io_uv - 0.5f) < 0.5f)
        color = u_color;
    else
        color = vec4(0.0f);
}
)";

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
auto renderer::quad2d(glm::vec2 const& position, glm::vec2 const& size, glm::vec4 const& color) -> void {
    m_quad_shader->bind();
    m_quad_shader->upload("u_color", color);
    auto model = glm::translate(glm::mat4{1.0f}, glm::vec3{position, 0.0f});
    model = glm::scale(model, glm::vec3{size, 1.0f});
    m_quad_shader->upload("u_model", model);
    m_quad_shader->upload("u_view", m_camera->view());
    m_quad_shader->upload("u_projection", m_camera->projection());
    m_quad_vertex->bind();
    m_quad_index->bind();
    glDrawElements(GL_TRIANGLES, m_quad_index->size(), m_quad_index->type(), nullptr);
}
auto renderer::line2d(glm::vec2 const& a, glm::vec2 const& b, glm::vec4 const& color, float const& thickness) -> void {
    auto const diff   = b - a;
    auto const length = glm::length(diff);
    auto const angle  = glm::acos(diff.x / length) * (diff.y > 0 ? 1.0f : -1.0f);
    m_quad_shader->bind();
    m_quad_shader->upload("u_color", color);
    auto model = glm::translate(glm::mat4{1.0f}, glm::vec3{a, 0.0f});
    model = glm::rotate(model, angle, {0.0f, 0.0f, 1.0f});
    model = glm::scale(model, glm::vec3{length, thickness, 1.0f});
    model = glm::translate(model, glm::vec3{0.5f, 0.0f, 0.0f});
    m_quad_shader->upload("u_model", model);
    m_quad_shader->upload("u_view", m_camera->view());
    m_quad_shader->upload("u_projection", m_camera->projection());
    m_quad_vertex->bind();
    m_quad_index->bind();
    glDrawElements(GL_TRIANGLES, m_quad_index->size(), m_quad_index->type(), nullptr);
}
auto renderer::circle2d_fill(glm::vec2 const& position, glm::vec2 const& size, glm::vec4 const& color) -> void {
    auto model = glm::translate(glm::mat4{1.0f}, glm::vec3{position, 0.0f});
    model = glm::scale(model, glm::vec3{size, 0.0f});
    m_circle_shader->bind();
    m_circle_shader->upload("u_color", color);
    m_circle_shader->upload("u_model", model);
    m_circle_shader->upload("u_view", m_camera->view());
    m_circle_shader->upload("u_projection", m_camera->projection());
    m_quad_vertex->bind();
    m_quad_index->bind();
    glDrawElements(GL_TRIANGLES, m_quad_index->size(), m_quad_index->type(), nullptr);
}
auto renderer::end() -> void {
    // TODO: clean up
}

auto renderer::begin_imgui() -> void {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
auto renderer::begin_dockspace() -> void {
    m_dockspace_flags = ImGuiDockNodeFlags_None;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    ImGuiViewport const* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar
                 |  ImGuiWindowFlags_NoCollapse
                 |  ImGuiWindowFlags_NoResize
                 |  ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus
                 |  ImGuiWindowFlags_NoNavFocus;
    m_dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", nullptr, window_flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    // Submit the DockSpace
    m_dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(m_dockspace_id, ImVec2(0.0f, 0.0f), m_dockspace_flags);
}
auto renderer::end_dockspace() -> void { ImGui::End(); }
auto renderer::end_imgui() -> void {
    ImGui::Render();
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

    static vertex quad_vertices[] {
        {{-0.5f,  0.5f,  0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    };
    static std::uint32_t quad_indices[] {
        0, 1, 2,
        0, 2, 3
    };

    m_quad_shader = shelter::make_local<shader>(DEFAULT_VERTEX_SHADER, DEFAULT_FRAGMENT_SHADER);
    m_quad_vertex = shelter::make_vertex_buffer_local(m_context, quad_vertices, sizeof(quad_vertices), {
        {shelter::data_type::vec3, "a_position"},
        {shelter::data_type::vec4, "a_color"},
        {shelter::data_type::vec2, "a_uv"},
    });
    m_quad_index  = shelter::make_index_buffer_local(m_context, quad_indices, sizeof(quad_indices),
        static_cast<std::uint32_t>(shelter::length_of(quad_indices)));

    m_circle_shader = shelter::make_local<shader>(DEFAULT_VERTEX_SHADER, CIRCLE_FRAGMENT_SHADER);
}
} // namespace shelter
