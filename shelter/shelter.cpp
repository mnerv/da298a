/**
 * @file   shelter.cpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  Shelter program
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
#include "sky.hpp"

#include "asio.hpp"

namespace shelter {
using sockets_t = std::vector<std::shared_ptr<asio::ip::tcp::socket>>;

using mac_address_t = std::uint8_t[6];

struct node {
    mac_address_t id;
    mac_address_t edges[4];
};

struct link_layer {
    std::uint8_t  preamble;
    mac_address_t destination;
    mac_address_t source;
    std::uint8_t  payload[10];
    std::uint8_t  crc;
};
}

auto main([[maybe_unused]]int argc, [[maybe_unused]]char const* argv[]) -> int {
    using asio::ip::tcp;
    using namespace std::chrono_literals;

    asio::io_context io_context;
    asio::io_context::work idle_work{io_context};
    asio::ip::port_type const port = 3000;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
    std::thread thread_context([&]{ io_context.run(); });

    std::mutex socket_mutex;
    shelter::sockets_t sockets;

    std::atomic<bool> listen_running = true;
    std::thread listen_thread([&]{
        fmt::print("shelter@{}:{}\n", acceptor.local_endpoint().address().to_string(), acceptor.local_endpoint().port());
        while(listen_running) {
            auto socket = std::make_shared<tcp::socket>(io_context);
            acceptor.accept(*socket);
            std::scoped_lock lock(socket_mutex);
            sockets.push_back(socket);
        }
    });

    std::atomic<bool> broadcast_running = true;
    std::atomic<bool> should_broadcast  = false;
    std::thread broadcast_thread([&]{
        fmt::print("Starting broadcasting to all the nodes!\n");
        while (broadcast_running) {
            if (should_broadcast)
                fmt::print("Broadcasting\n");
            asio::steady_timer t(io_context, 500ms);
            t.wait();
        }
    });

    fmt::print("Starting main simulation...\n");
    auto is_running = true;
    while (is_running) {
        asio::steady_timer t(io_context, 1s);
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::time_point_cast<std::chrono::seconds>(now);
        fmt::print("Simulation tick: {}\n", time.time_since_epoch().count());
        should_broadcast = !should_broadcast;
        t.wait();
    }

    broadcast_thread.join();
    listen_thread.join();
    io_context.stop();
    thread_context.join();
    return 0;
}
