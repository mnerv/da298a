/**
 * @file   window.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  window abstraction
 * @date   2022-11-08
 *
 * @copyright Copyright (c) 2022
 */
#ifndef SHELTER_WINDOW_HPP
#define SHELTER_WINDOW_HPP

#include <string>
#include <limits>
#include "common.hpp"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "glm/vec2.hpp"

namespace shelter {
struct window_props {
    std::string  title  = "shelter::window";
    std::int32_t width  = 738;
    std::int32_t height = 480;
    std::int32_t xpos{std::numeric_limits<std::int32_t>::min()};
    std::int32_t ypos{std::numeric_limits<std::int32_t>::min()};
};
auto make_window(window_props const& props) -> window_ref_t;

class window {
public:
    window(window_props const& props);
    ~window();

    [[nodiscard]]auto title()  const -> std::string const& { return m_data.title; }
    [[nodiscard]]auto width()  const -> std::int32_t { return m_data.width; }
    [[nodiscard]]auto height() const -> std::int32_t { return m_data.height; }
    [[nodiscard]]auto native() const -> GLFWwindow* { return m_window; }
    [[nodiscard]]auto context() const -> graphics_context_ref_t const&;
    [[nodiscard]]auto renderer() const -> renderer_ref_t const&;

    auto set_title(std::string const& title) -> void;
    auto set_width(std::int32_t const& width) -> void;
    auto set_height(std::int32_t const& height) -> void;
    auto set_context(graphics_context_ref_t context) -> void;
    auto set_renderer(renderer_ref_t renderer) -> void;

    [[nodiscard]]auto shouldclose() const -> bool;
    [[nodiscard]]auto key(std::int32_t const& key) const -> std::int32_t;
    [[nodiscard]]auto mouse_pos() const -> glm::dvec2;
    auto poll() -> void;

private:
    GLFWwindow* m_window;
    graphics_context_ref_t m_context{nullptr};
    renderer_ref_t         m_renderer{nullptr};

    struct data {
        std::string  title;
        std::int32_t width;
        std::int32_t height;
    } m_data;
};
} // namespace shelter


#endif  // SHELTER_WINDOW_HPP
