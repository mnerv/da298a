/*
 * @file   renderer.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  renderer abstraction
 * @date   2022-11-08
 *
 * @copyright Copyright (c) 2022
 */
#ifndef SHELTER_RENDERER_HPP
#define SHELTER_RENDERER_HPP

#include "common.hpp"
#include "window.hpp"
#include "graphics_context.hpp"

#include "shader.hpp"
#include "buffer.hpp"
#include "camera.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "glm/mat4x4.hpp"

namespace shelter {
auto make_renderer(graphics_context_ref_t context) -> renderer_ref_t;

class renderer {
public:
    renderer(graphics_context_ref_t context);
    ~renderer();

    auto context() const -> graphics_context_ref_t const& { return m_context; }

    auto begin(camera_ref_t const& camera) -> void;
    auto submit(shader_ref_t const& shader, vertex_buffer_ref_t const& vb, index_buffer_ref_t const& ib, glm::mat4 const& model) -> void;
    auto quad2d(glm::vec2 const& position, glm::vec2 const& size, glm::vec4 const& color) -> void;
    auto line2d(glm::vec2 const& a, glm::vec2 const& b, glm::vec4 const& color, float const& thickness = 1.0f) -> void;
    auto circle2d_fill(glm::vec2 const& position, glm::vec2 const& size, glm::vec4 const& color) -> void;
    auto end() -> void;

    auto begin_imgui() -> void;
    auto begin_dockspace() -> void;
    auto end_dockspace() -> void;
    auto end_imgui() -> void;

    auto dockspace_id() -> ImGuiID { return m_dockspace_id; }
    auto dockspace_flags() -> ImGuiDockNodeFlags { return m_dockspace_flags; }

private:
    graphics_context_ref_t m_context;
    camera_ref_t           m_camera;

    shader_local_t        m_circle_shader{nullptr};
    shader_local_t        m_quad_shader{nullptr};

    index_buffer_local_t  m_quad_index{nullptr};
    vertex_buffer_local_t m_quad_vertex{nullptr};

    ImGuiDockNodeFlags    m_dockspace_flags;
    ImGuiID               m_dockspace_id;

private:
    auto setup_2d() -> void;
};
} // namespace shelter

#endif  // SHELTER_RENDERER_HPP

