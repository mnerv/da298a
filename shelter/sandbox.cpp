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
#include "flicker.hpp"

#include "glm/gtc/type_ptr.hpp"
#include "glm/vec4.hpp"

template <typename T>
struct input_state {
    T current;
    T previous;

    auto update(T const& value) {
        previous = current;
        current  = value;
    }
    auto is_switched() const -> bool {
        return current != previous;
    }
};

auto entry() -> int {
    using asio::ip::tcp;
    using namespace std::chrono_literals;

    auto window   = shelter::make_window({"Shelter Sandbox"});
    auto context  = shelter::make_graphics_context(window);
    auto renderer = shelter::make_renderer(context);

    auto time = window->time();
    auto previous_time = time;
    glm::vec4 clear_color{0.058f};
    glm::vec3 camera_saved_position{};
    glm::vec2 mouse_start{};
    input_state<bool> mouse_press{false, false};
    input_state<bool> mouse_pan{false, false};
    input_state<bool> mouse_line{false, false};
    auto camera = shelter::make_camera();

    auto mouse_pos = window->mouse_pos();
    auto cursor_world_position = [&] {
        return glm::vec2(mouse_pos.x, -mouse_pos.y) - glm::vec2(window->width() / 2, -window->height() / 2)
               + glm::vec2(camera->position().x, camera->position().y);
    };

    flicker::app app;

    auto is_running = true;
    while (is_running) {
        previous_time = time;
        time = window->time();

        is_running = !window->shouldclose();
        if (window->is_key_down(GLFW_KEY_Q))
            is_running = false;
        if (window->is_key_down(GLFW_KEY_0))
            camera->set_position(glm::vec3{0.0f});
        mouse_pos = window->mouse_pos();

        mouse_press.update(window->is_mouse_down(GLFW_MOUSE_BUTTON_1) && !ImGui::IsAnyItemFocused());
        mouse_pan.update(window->is_mouse_down(GLFW_MOUSE_BUTTON_1) && window->is_key_down(GLFW_KEY_LEFT_ALT));
        if (mouse_pan.current && mouse_pan.is_switched()) {
            mouse_start = window->mouse_pos();
            camera_saved_position = camera->position();
        } else if(mouse_pan.current) {
            auto const current = window->mouse_pos();
            auto const delta = glm::vec3(-(current.x - mouse_start.x), current.y - mouse_start.y, 0.0f);
            camera->set_position(camera_saved_position + delta);
        }

        camera->update(window);

        context->viewport(0, 0, std::uint32_t(window->buffer_width()), std::uint32_t(window->buffer_height()));
        context->set_clear_color(clear_color);
        context->clear();

        renderer->begin(camera);
        // for (float i = 0.0f; i < 2.0f; ++i) {
        //     for (float j = 0.0f; j < 2.0f; ++j) {
        //         float const offset  = 20.0f;
        //         float const padding = 50.0f;
        //         glm::vec2 position{(offset + padding) * j, (offset + padding) * i};
        //         renderer->quad2d(position, {20.0f, 20.0f}, glm::vec4{1.0f});
        //     }
        // }
        auto const& graph = app.graph();
        for (std::size_t i = 0; i < graph.size(); ++i) {
            float const offset  = 20.0f;
            float const padding = 50.0f;
            glm::vec2 position{(offset + padding) * i, 0.0f};
            renderer->quad2d(position, {20.0f, 20.0f}, glm::vec4{1.0f});
        }

        renderer->circle2d_fill(cursor_world_position(), {10.0f, 10.0f}, {1.0f, 0.0f, 0.0f, 1.0f});
        renderer->end();

        renderer->begin_imgui();
        ImGui::SetNextWindowPos({float(window->xpos()) + 2.0f, float(window->ypos()) + 2.0f}, ImGuiCond_Once);
        ImGui::Begin("settings");
        auto const frametime = time - previous_time;
        ImGui::Text("%s", fmt::format("frametime: {:#.3f}s, {:#.2f} fps", frametime, 1.0f / frametime).c_str());
        ImGui::ColorEdit3("clear", glm::value_ptr(clear_color));
        ImGui::Separator();

        if (ImGui::Button("start"))
            app.start();
        ImGui::SameLine();
        if (ImGui::Button("stop"))
            app.stop();
        ImGui::SameLine();
        ImGui::Text("%s", app.is_running() ? "running" : "stopped");

        if (ImGui::Button("create"))
            app.create_hardware();
        ImGui::End();

        renderer->end_imgui();

        context->swap();
        window->poll();
    }

    fmt::print("Goodbye world...\n");
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
