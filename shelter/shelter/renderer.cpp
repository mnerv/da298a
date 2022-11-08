/*
 * @file   renderer.cpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  renderer abstraction
 * @date   2022-11-08
 *
 * @copyright Copyright (c) 2022
 */
#include "renderer.hpp"

namespace shelter {
auto make_renderer(graphics_context_ref_t context) -> renderer_ref_t {
    return make_ref<renderer>(context);
}

renderer::renderer(graphics_context_ref_t context) : m_context(std::move(context)) {
}
renderer::~renderer() = default;
} // namespace shelter
