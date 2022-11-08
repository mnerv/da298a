/*
 * @file   camera.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  camera
 * @date   2022-11-08
 *
 * @copyright Copyright (c) 2022
 */
#ifndef SHELTER_CAMERA_HPP
#define SHELTER_CAMERA_HPP

#include "common.hpp"

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

namespace shelter {
auto make_camera() -> camera_ref_t;

class camera {
public:
    camera();
    ~camera();

    auto update(window_ref_t const& window) -> void;
    auto view() const -> glm::mat4 { return m_view; }
    auto projection() const -> glm::mat4;

private:
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::mat4 m_view;

    glm::vec2 m_size;
};
} // namespace shelter

#endif  // SHELTER_CAMERA_HPP
