/**
 * @file   shelter.cpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  Shelter sandbox simulation program.
 * @date   2022-10-18
 *
 * @copyright Copyright (c) 2022
 */
#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <array>
#include <vector>
#include <memory>
#include <atomic>
#include <cmath>

#include "fmt/format.h"
#include "asio.hpp"

#include "shelter/shelter.hpp"
#include "sky.hpp"

#include "glm/gtc/type_ptr.hpp"
#include "glm/vec4.hpp"

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

template <typename T>
struct input_state {
    T current;
    T previous;

    auto update(T const& value) {
        previous = current;
        current = value;
    }

    auto is_switched() const -> bool {
        return current != previous;
    }
};

auto entry() -> int {
    auto window   = shelter::make_window({"Shelter Sandbox"});
    auto context  = shelter::make_graphics_context(window);
    auto renderer = shelter::make_renderer(context);

    auto shader = shelter::make_shader(context);
    auto ib = shelter::make_index_buffer(context, indices, sizeof(indices),
        static_cast<std::uint32_t>(shelter::length_of(indices)));
    auto vb = shelter::make_vertex_buffer(context, vertices, sizeof(vertices), {
        {shelter::data_type::vec3, "a_position"},
        {shelter::data_type::vec4, "a_color"},
        {shelter::data_type::vec2, "a_uv"},
    });

    auto camera = shelter::make_camera();
    glm::vec4 clear_color{0.058f};

    glm::vec3 camera_saved_position{};
    glm::vec2 mouse_start{};
    input_state<bool> mouse_press{false, false};

    auto is_running = true;
    while (is_running) {
        is_running = !window->shouldclose();
        if (window->key(GLFW_KEY_Q) == GLFW_PRESS)
            is_running = false;

        mouse_press.update(window->mouse(GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !ImGui::IsAnyItemActive());
        if (mouse_press.current && mouse_press.is_switched()) {
            mouse_start = window->mouse_pos();
            camera_saved_position = camera->position();
        } else if(mouse_press.current) {
            auto const current = window->mouse_pos();
            auto const delta = glm::vec3(-(current.x - mouse_start.x), current.y - mouse_start.y, 0.0f);
            camera->set_position(camera_saved_position + delta);
        }

        camera->update(window);
        context->viewport(0, 0, window->buffer_width(), window->buffer_height());
        context->set_clear_color(clear_color);
        context->clear();

        renderer->begin(camera);
        shader->upload("u_color", glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
        for (float i = 0.0f; i < 16.0f; ++i) {
            for (float j = 0.0f; j < 16.0f; ++j) {
                float const offset  = 20.0f;
                float const padding = 25.0f;
                glm::vec3 position{(offset + padding) * j, (offset + padding) * i, 1.0f};
                renderer->submit(shader, vb, ib, glm::scale(glm::translate(glm::mat4{1.0f}, position), glm::vec3{20.0f}));
            }
        }
        renderer->end();

        renderer->begin_imgui();
        ImGui::SetNextWindowSize({256.0f, 60.0f}, ImGuiCond_Once);
        ImGui::SetNextWindowPos({float(window->posx()) + 2.0f, float(window->posy()) + 2.0f}, ImGuiCond_Once);
        ImGui::Begin("settings");
        ImGui::ColorEdit3("clear", glm::value_ptr(clear_color));
        ImGui::End();
        ImGui::Render();
        renderer->end_imgui();

        context->swap();
        window->poll();
    }
    return 0;
}

auto main([[maybe_unused]]int argc, [[maybe_unused]]char const* argv[]) -> int {
    try {
        return entry();
    } catch(std::runtime_error const& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}
