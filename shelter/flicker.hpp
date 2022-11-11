/**
 * @file    flicker.hpp
 * @author  Pratchaya Khansomboon (me@mononerv.dev)
 * @brief   Simulation emulation
 * @version 0.0
 * @date    2022-11-11
 * 
 * @copyright Copyright (c) 2022
 */

#ifndef SHELTER_FLICKER_HPP
#define SHELTER_FLICKER_HPP

#include <thread>
#include <atomic>
#include <mutex>
#include <memory>
#include <vector>

#include "shelter/utility.hpp"
#include "asio.hpp"
#include "fmt/format.h"

namespace flicker {
struct hardware {
    std::uint32_t               id{};
    std::atomic<bool>           is_running{false};
    shelter::local<std::thread> thread{nullptr};

    auto loop() -> void {
        // fmt::print("{:#04x}: Hello, World!\n", id);
    }
};

using acceptor_t   = asio::ip::tcp::acceptor;
using socket_t     = asio::ip::tcp::socket;
using socket_ref_t = shelter::ref<socket_t>;
using hardware_ref_t = shelter::ref<hardware>;

auto emulation_loop(hardware_ref_t hw) -> void {
    using namespace std::chrono_literals;
    hw->is_running = true;

    asio::io_context io;
    asio::ip::tcp::resolver resolver(io);
    auto endpoints = resolver.resolve("localhost", "3000");
    asio::ip::tcp::socket socket(io);
    asio::connect(socket, endpoints);
    fmt::print("shelter::hardware ID[{:#02d}] connected!\n", hw->id);

    while (hw->is_running) {
        asio::steady_timer t(io, 1s);
        hw->loop();
        t.wait();
    }

    fmt::print("shelter::hardware ID[{:#02d}] disconnected!\n", hw->id);
}


struct node {
    using id_t    = std::uint32_t;
    using edges_t = std::uint32_t[4];

    id_t           id;
    edges_t        edges;
    hardware_ref_t hw;
};

class app {
public:
    app() = default;
    ~app() {
        stop();
        if (m_close != nullptr && m_close->joinable()) m_close->join();
    }

    auto start() -> void {
        if (m_is_running || m_is_closing) return;
        if (m_close != nullptr) m_close->join();
        m_is_running = true;
        using asio::ip::tcp;
        m_thread    = shelter::make_local<std::thread>([this] { m_io.run(); });
        m_acceptor  = shelter::make_ref<acceptor_t>(m_io, tcp::endpoint(tcp::v4(), 3000));

        m_listen = shelter::make_local<std::thread>([this] {
            fmt::print("flicker::app@{}:{}\n",
                       m_acceptor->local_endpoint().address().to_string(),
                       m_acceptor->local_endpoint().port());
            while(m_is_running) {
                try {
                    auto socket = shelter::make_ref<socket_t>(m_io);
                    m_acceptor->accept(*socket);
                    std::scoped_lock lock(m_socket_mutex);
                    m_sockets.push_back(socket);
                } catch (asio::system_error const& e) {
                    if (m_is_running) fmt::print("{}\n", e.what());
                }
            }
        });

        m_receive = shelter::make_local<std::thread>([this] {
            while(m_is_running) {
                asio::steady_timer t(m_io, std::chrono::seconds(1));
                t.wait();
            }
        });

        m_send = shelter::make_local<std::thread>([this] {
            while(m_is_running) {
                asio::steady_timer t(m_io, std::chrono::seconds(1));
                t.wait();
            }
        });
    }

    auto create_hardware() -> void {
        if (!m_is_running || m_is_closing) return;
        auto hw = shelter::make_ref<hardware>();
        hw->id = std::uint32_t(m_hardwares.size());
        m_hardwares.push_back(hw);
        hw->thread = shelter::make_local<std::thread>(emulation_loop, hw);
    }

    auto stop() -> void {
        if (!m_is_running || m_is_closing) return;
        m_is_running = false;
        m_is_closing = true;

        m_close = shelter::make_local<std::thread>([&]{
            for (auto const& hw : m_hardwares) {
                hw->is_running = false;
                hw->thread->join();
            }
            m_hardwares = {};

            m_acceptor->close();
            m_listen->join();
            m_receive->join();
            m_send->join();
            m_io.stop();
            m_thread->join();
            m_is_closing = false;
            fmt::print("flicker::app stopped.\n");
        });
    }

    auto is_running() -> bool { return m_is_running; }
    auto graph() const -> std::array<node, 4> const& { return m_graph; }

private:
    asio::io_context m_io{};
    std::mutex       m_socket_mutex{};
    shelter::local<std::thread> m_thread{nullptr};
    shelter::local<std::thread> m_close{nullptr};
    shelter::local<std::thread> m_listen{nullptr};
    shelter::local<std::thread> m_receive{nullptr};
    shelter::local<std::thread> m_send{nullptr};
    shelter::ref<acceptor_t>    m_acceptor{nullptr};

    std::atomic<bool>           m_is_running{false};
    std::atomic<bool>           m_is_closing{false};
    std::vector<socket_ref_t>   m_sockets{};
    std::vector<hardware_ref_t> m_hardwares{};

    std::array<node, 4> m_graph{{
        {1, {3, 2, 0, 0}, nullptr},
        {2, {4, 0, 0, 1}, nullptr},
        {3, {0, 4, 1, 0}, nullptr},
        {4, {0, 0, 2, 3}, nullptr},
    }};
};
} // namespace flicker


#endif  // SHELTER_FLICKER_HPP
