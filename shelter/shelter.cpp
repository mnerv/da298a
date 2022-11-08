/**
 * @file   shelter.cpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  Shelter is a sandbox simulation program.
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
using socket_ref_t = std::shared_ptr<asio::ip::tcp::socket>;
using sockets_t    = std::vector<socket_ref_t>;

auto hardware_loop(std::uint32_t id) {
    asio::io_context io;
    asio::ip::tcp::resolver resolver(io);
    auto endpoints = resolver.resolve("localhost", "3000");
    asio::ip::tcp::socket socket(io);
    asio::connect(socket, endpoints);

    fmt::print("shelter::hardware ID[{:#02d}] connected!\n", id);
    constexpr std::size_t buffer_size = 128;
    std::array<char, buffer_size> buf;
    buf[0] = char(id >> 24);
    buf[1] = char(id >> 16);
    buf[2] = char(id >>  8);
    buf[3] = char(id >>  0);

    socket.send(asio::buffer(buf));

    for (;;) {
        asio::error_code err;
        std::size_t len = socket.read_some(asio::buffer(buf), err);

        if (err == asio::error::eof)
            break;
        else if (err)
            break;
            // throw asio::system_error(err);

        std::string str(buf.data(), len);
        fmt::print("shelter::hardware ID[{:#02d}] receive: {}", id, str);
        // std::cout.write(buf.data(), std::int64_t(len));
    }

    fmt::print("shelter::hardware ID[{:#02d}] shutdown...\n", id);
}
}

auto main([[maybe_unused]]int argc, [[maybe_unused]]char const* argv[]) -> int {
    using asio::ip::tcp;
    using namespace std::chrono_literals;

    fmt::print("shelter program\n");

    asio::io_context io_context;
    asio::io_context::work idle_work{io_context};
    asio::ip::port_type const port = 3000;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
    std::thread thread_context([&]{ io_context.run(); });

    std::mutex socket_mutex;
    shelter::sockets_t sockets;

    std::atomic<bool> is_listen_running = true;
    std::thread listen_thread([&]{
        fmt::print("shelter@{}:{}\n", acceptor.local_endpoint().address().to_string(),
                                      acceptor.local_endpoint().port());
        while(is_listen_running) {
            try {
                auto socket = std::make_shared<tcp::socket>(io_context);
                acceptor.accept(*socket);
                std::scoped_lock lock(socket_mutex);
                sockets.push_back(socket);
            } catch (asio::system_error const& e) {
                if (is_listen_running) fmt::print("{}\n", e.what());
            }
        }
    });

    std::atomic<bool> is_send_running = true;
    std::atomic<bool> should_send     = false;
    std::thread send_thread([&]{
        fmt::print("Starting broadcasting to all the nodes!\n");
        while (is_send_running) {
            if (should_send) {
                fmt::print("Broadcasting...\n");
                std::scoped_lock lock(socket_mutex);
                for (auto const& socket : sockets) {
                    auto msg = fmt::format("ping\n");
                    asio::error_code ignore_err;
                    asio::write(*socket, asio::buffer(msg), ignore_err);
                }
            }
            asio::steady_timer t(io_context, 500ms);
            t.wait();

            // TODO: Send close message
            if (!is_send_running) for (auto const& socket : sockets) socket->close();
        }
    });

    std::atomic<bool> is_receive_running = true;
    std::thread receive_thread([&]{
        fmt::print("Start receiving from all the nodes!\n");
        constexpr std::size_t buffer_size = 256;
        while(is_receive_running) {
            asio::steady_timer t(io_context, 1s);
            for (auto const& socket : sockets) {
                std::array<char, buffer_size> buf;
                asio::error_code err;
                try {
                    [[maybe_unused]]auto len = socket->read_some(asio::buffer(buf), err);

                    std::uint32_t id{0};
                    id |= std::uint32_t(buf[0] << 24);
                    id |= std::uint32_t(buf[1] << 16);
                    id |= std::uint32_t(buf[2] <<  8);
                    id |= std::uint32_t(buf[3] <<  0);
                    fmt::print("{}:{} {}\n", socket->remote_endpoint().address().to_string(),
                                             socket->remote_endpoint().port(), id);
                } catch(asio::system_error const& e) {
                    if (is_receive_running) fmt::print("{}\n", e.what());
                }
            }
            // TODO: Handle receiving message
            t.wait();
        }
    });

    std::thread hardware_loop_thread([]{
        std::vector<std::shared_ptr<std::thread>> threads;
        std::uint32_t id = 0;
        constexpr std::size_t hardware_count = 16;
        fmt::print("creating hardware sims\n");
        for (std::size_t i = 0; i < hardware_count; ++i)
            threads.emplace_back(std::make_shared<std::thread>(shelter::hardware_loop, id++));
        for (auto const& t : threads)
            t->join();
    });

    fmt::print("Starting main simulation...\n");
    auto is_running = true;
    auto steps = 0;
    while (is_running) {
        asio::steady_timer t(io_context, 1s);
        auto now  = std::chrono::system_clock::now();
        auto time = std::chrono::time_point_cast<std::chrono::seconds>(now);
        fmt::print("Simulation tick: {}, {}\n", time.time_since_epoch().count(), steps);
        should_send = !should_send;
        t.wait();
        ++steps;

        // FIXME: Control stop with button
        if (steps > 256) {
            is_running         = false;
            is_listen_running  = false;
            is_send_running    = false;
            is_receive_running = false;
            acceptor.cancel();
        }
    }

    hardware_loop_thread.join();
    receive_thread.join();
    send_thread.join();
    listen_thread.join();
    io_context.stop();
    thread_context.join();

    fmt::print("Goodbye world...\n");
    return 0;
}
