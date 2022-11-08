/*
 * @file   camera.cpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  camera
 * @date   2022-11-08
 *
 * @copyright Copyright (c) 2022
 */
#include "camera.hpp"
#include "window.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace shelter {
auto make_camera() -> camera_ref_t {
    return make_ref<camera>();
}

camera::camera() {
    m_position = { 0.0f,  0.0f,  1.0f};
    m_front    = { 0.0f,  0.0f, -1.0f};
    m_up       = { 0.0f,  1.0f,  0.0f};
    m_view = glm::lookAt(m_position, m_position + m_front, m_up);
}
camera::~camera() = default;

auto camera::projection(window_ref_t const& window) const -> glm::mat4 {
    auto const width  = static_cast<float>(window->width());
    auto const height = static_cast<float>(window->height());
    return glm::ortho(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f);
}
} // namespace shelter
