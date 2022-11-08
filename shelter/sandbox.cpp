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

#include "shelter/window.hpp"
#include "sky.hpp"

#include "glad/glad.h"

auto main([[maybe_unused]]int argc, [[maybe_unused]]char const* argv[]) -> int {
    auto window = shelter::make_window({"Shelter Sandbox"});

    auto is_running = true;
    while (is_running) {
        is_running = !window->shouldclose();
        if (window->key(GLFW_KEY_Q) == GLFW_PRESS)
            is_running = false;

        window->begin_imgui();

        ImGui::Begin("Hello, World!");
        ImGui::Button("much wow");
        ImGui::End();

        ImGui::Render();
        glViewport(0, 0, window->width(), window->height());
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        window->end_imgui();
        window->poll();
        window->swap();
    }
    return 0;
}
