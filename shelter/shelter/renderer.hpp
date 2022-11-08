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

namespace shelter {
auto make_renderer(graphics_context_ref_t context) -> renderer_ref_t;

class renderer {
public:
    renderer(graphics_context_ref_t context);
    ~renderer();

    auto context() const -> graphics_context_ref_t const& { return m_context; }

    auto begin_imgui() const -> void;
    auto end_imgui() const -> void;

private:
    graphics_context_ref_t m_context;
};
} // namespace shelter

#endif  // SHELTER_RENDERER_HPP

