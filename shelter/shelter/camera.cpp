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
    m_size = {};
}
camera::~camera() = default;

auto camera::update(window_ref_t const& window) -> void {
    m_size.x = static_cast<float>(window->width());
    m_size.y = static_cast<float>(window->height());
}
auto camera::projection() const -> glm::mat4 {
    return glm::ortho(-m_size.x / 2.0f, m_size.x / 2.0f, -m_size.y / 2.0f, m_size.y / 2.0f);
}
} // namespace shelter
