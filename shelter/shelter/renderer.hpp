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
    auto quad(glm::vec2 const& position, glm::vec2 const& size, glm::vec4 const& color) -> void;
    auto end() -> void;

    auto begin_imgui() const -> void;
    auto end_imgui() const -> void;

private:
    graphics_context_ref_t m_context;
    camera_ref_t           m_camera;

    struct renderer_2d{
        shader_ref_t        shader{nullptr};
        index_buffer_ref_t  index_buffer{nullptr};
        vertex_buffer_ref_t vertex_buffer{nullptr};
    };
    renderer_2d m_2d;

private:
    auto setup_2d() -> void;
};
} // namespace shelter

#endif  // SHELTER_RENDERER_HPP

