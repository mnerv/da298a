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

auto main([[maybe_unused]]int argc, [[maybe_unused]]char const* argv[]) -> int {
    auto window   = shelter::make_window({"Shelter Sandbox"});
    auto context  = shelter::make_graphics_context(window);
    auto renderer = shelter::make_renderer(context);

    vertex vertices[] {
        {{-1.0f,  1.0f,  0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{ 1.0f,  1.0f,  0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{ 1.0f, -1.0f,  0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        {{-1.0f, -1.0f,  0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    };
    std::uint32_t indices[] {
        0, 1, 2,
        0, 2, 3
    };

    auto shader = shelter::make_shader(context);
    auto ib = shelter::make_index_buffer(context, indices, sizeof(indices),
        static_cast<std::int32_t>(shelter::length_of(indices)));
    auto vb = shelter::make_vertex_buffer(context, vertices, sizeof(vertices), {
        {shelter::data_type::vec3, "a_position"},
        {shelter::data_type::vec4, "a_color"},
        {shelter::data_type::vec2, "a_uv"},
    });

    glm::vec4 clear_color{0.058f, 0.058f, 0.058f, 1.0f};
    auto is_running = true;
    while (is_running) {
        is_running = !window->shouldclose();
        if (window->key(GLFW_KEY_Q) == GLFW_PRESS)
            is_running = false;

        context->viewport(0, 0, window->width(), window->height());
        context->set_clear_color(clear_color);
        context->clear();

        // renderer->begin();
        // renderer->submit();
        // renderer->end();

        shader->bind();
        vb->bind();
        ib->bind();
        glDrawElements(GL_TRIANGLES, ib->size(), ib->type(), nullptr);

        renderer->begin_imgui();
        ImGui::SetNextWindowSize({256.0f, 60.0f}, ImGuiCond_FirstUseEver);
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
