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

#include "fmt/format.h"
#include "sky.hpp"

#define ASIO_STANDALONE
#include "asio.hpp"

auto client_test() -> void {
    asio::io_context io;
    asio::ip::tcp::resolver resolver(io);
    auto endpoints = resolver.resolve("localhost", "daytime");
    asio::ip::tcp::socket socket(io);
    asio::connect(socket, endpoints);

    fmt::print("connected to {}\n", socket.remote_endpoint().address().to_string());

    for (;;) {
        std::array<char, 128> buf;
        asio::error_code err;
        std::size_t len = socket.read_some(asio::buffer(buf), err);

        if (err == asio::error::eof)
            break;
        else if (err)
            throw asio::system_error(err);

        std::cout.write(buf.data(), len);
    }

    fmt::print("Goodbye world...\n");
}

auto main([[maybe_unused]]int argc, [[maybe_unused]]char const* argv[]) -> int {
    using asio::ip::tcp;

    asio::io_context io_context;
    asio::io_context::work idle_work{io_context};
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 13));
    std::thread thread_context([&]{ io_context.run(); });
    std::vector<std::shared_ptr<tcp::socket>> sockets;

    std::thread listen_thread([&]{
        for (;;) {
            auto socket = std::make_shared<tcp::socket>(io_context);
            acceptor.accept(*socket);
            sockets.push_back(socket);
        }
    });

    std::thread clients([]{
        std::vector<std::shared_ptr<std::thread>> threads;
        for (auto i = 0; i < 0x10; i++)
            threads.emplace_back(std::make_shared<std::thread>(client_test));
        for (auto const& t : threads)
            t->join();
    });

    fmt::print("Sleeping for 5s\n");
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::thread broadcast_thread([&]{
        fmt::print("Starting broadcasting to all the nodes!\n");
        auto i = 0;
        for (auto socket : sockets) {
            auto msg = fmt::format("This is a message for you all: {}\n", i++);
            asio::error_code ignore_err;
            asio::write(*socket, asio::buffer(msg), ignore_err);
        }
    });

    clients.join();
    broadcast_thread.join();
    listen_thread.join();
    io_context.stop();
    thread_context.join();
    return 0;
}
