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

auto main([[maybe_unused]]int argc, [[maybe_unused]]char const* argv[]) -> int {
    auto window   = shelter::make_window({"Shelter Sandbox"});
    auto context  = shelter::make_graphics_context(window);
    auto renderer = shelter::make_renderer(context);

    glm::vec4 clear_color{0.058f, 0.058f, 0.058f, 1.0f};
    auto is_running = true;
    while (is_running) {
        is_running = !window->shouldclose();
        if (window->key(GLFW_KEY_Q) == GLFW_PRESS)
            is_running = false;

        renderer->begin_imgui();

        ImGui::SetNextWindowSize({256.0f, 60.0f}, ImGuiCond_FirstUseEver);
        ImGui::Begin("settings");
        ImGui::ColorEdit3("clear", glm::value_ptr(clear_color));
        ImGui::End();

        ImGui::Render();
        context->set_clear_color(clear_color);
        context->clear();

        renderer->end_imgui();
        window->poll();
        context->swap();
    }
    return 0;
}
