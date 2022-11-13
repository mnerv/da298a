/**
 * @file   shelter.cpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  Shelter sandbox simulation program.
 * @date   2022-10-18
 *
 * @copyright Copyright (c) 2022
 */
#include <iostream>
#include <stack>
#include <unordered_set>
#include <algorithm>

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
    auto camera = shelter::make_camera();

    auto mouse_pos = window->mouse_pos();
    auto cursor_world_position = [&] {
        return glm::vec2(mouse_pos.x, -mouse_pos.y) - glm::vec2(window->width() / 2, -window->height() / 2)
               + glm::vec2(camera->position().x, camera->position().y);
    };

    flicker::app app;

    using edges_t = std::array<std::uint32_t, 4>;
    struct node {
        std::uint32_t id;
        edges_t       edges;  // N, E, S, W
    };

    std::vector<node> graph{
    //   id    N   E   S   W
        { 1, { 0,  2,  7,  6}},
        { 2, { 3,  0,  0,  1}},
        { 3, { 4, 15,  2,  0}},
        { 4, { 0,  0,  4,  5}},
        { 5, { 0,  4,  0,  0}},
        { 6, { 0,  1,  0,  0}},
        { 7, { 1,  0,  8,  0}},
        { 8, { 7,  0,  0,  9}},
        { 9, { 0,  8,  0, 10}},
        {10, { 0,  9,  0, 11}},
        {11, {12, 10,  0,  0}},
        {12, {13,  0, 11,  0}},
        {13, { 0, 14, 12,  0}},
        {14, {16,  0,  0, 13}},
        {15, { 0,  0,  0,  3}},
        {16, { 0,  0, 14,  0}},
    };

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

        auto dfs_draw = [&renderer](std::vector<node> const& graph) {
            struct node_stack {
                std::uint32_t id;
                edges_t       edges;
                glm::vec2     position;
            };
            std::stack<node_stack>  stack;
            std::vector<node_stack> label;
            stack.push({
                graph[0].id,
                graph[0].edges,
                {0.0f, 0.0f},
            });
            while (!stack.empty()) {
                auto const v = stack.top(); stack.pop();
                auto const it = std::find_if(std::begin(label), std::end(label), [&v](auto const& s) { return s.id == v.id; });
                if (it == std::end(label)) {
                    auto const& edges = v.edges;
                    label.push_back(v);
                    float const offset = 60.0f;
                    float const size   = 25.0f;
                    renderer->circle2d_fill(v.position, glm::vec2{size}, glm::vec4{1.0f});
                    for (std::size_t i = 0; i < edges.size(); ++i) {
                        if (edges[i] == 0) continue;
                        auto current = v.position;
                        switch (i) {
                        case 0:  // North
                            current.y += offset;
                            break;
                        case 1:  // East
                            current.x += offset;
                            break;
                        case 2:  // South
                            current.y -= offset;
                            break;
                        case 3:  // West
                            current.x -= offset;
                            break;
                        }
                        renderer->line2d(v.position, current, glm::vec4{1.0f}, 1.0f);
                        auto const index = std::size_t(edges[i] - 1);
                        stack.push({
                            graph[index].id,
                            graph[index].edges,
                            current,
                        });
                    }
                }
            }
        };
        dfs_draw(graph);

        renderer->circle2d_fill({0.0f, 0.0f}, {12.0f, 12.0f}, {0.0f, 0.0f, 0.0f, 1.0f});
        renderer->circle2d_fill({0.0f, 0.0f}, {8.0f, 8.0f}, {1.0f, 1.0f, 1.0f, 1.0f});
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
