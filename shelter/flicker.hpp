/**
 * @file    flicker.hpp
 * @author  Pratchaya Khansomboon (me@mononerv.dev)
 * @author  Isac Pettersson
 * @author  Christian Heisterkamp
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
#include <queue>
#include <stdlib.h>


#include "shelter/utility.hpp"
#include "asio.hpp"
#include "fmt/format.h"
#include "glm/vec2.hpp"
#include "sky.hpp"

namespace flicker {

using edges_t = std::array<std::uint32_t, 4>;
struct node {
    std::uint32_t id;
    edges_t       edges;  // N, E, S, W
};

struct hardware {
    std::uint32_t               id{};
    std::atomic<bool>           is_running{false};
    shelter::local<std::thread> thread{nullptr};
    std::vector<node>           graph{};
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

    constexpr std::size_t buffer_size = 128;
    std::array<char, buffer_size> read_buffer{};
    std::array<char, buffer_size> send_buffer{};
    while (hw->is_running) {
        socket.async_read_some(asio::buffer(read_buffer),
            [&]([[maybe_unused]]asio::error_code const& error, [[maybe_unused]]std::size_t bytes_transferred) {
        });

        socket.async_send(asio::buffer(read_buffer),
            [&]([[maybe_unused]]asio::error_code const& error, [[maybe_unused]]std::size_t bytes_transferred) {
        });

        asio::steady_timer t(io, 1s);
        t.wait();
    }

    fmt::print("shelter::hardware ID[{:#02d}] disconnected!\n", hw->id);
}

constexpr std::size_t connection_buffer_size = 128;
using connection_buffer_t = std::array<char, connection_buffer_size>;
using connection_ref_t = shelter::ref<struct connection>;
struct connection {
    socket_ref_t        socket{nullptr};
    connection_buffer_t read_buffer{};
    connection_buffer_t send_buffer{};
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
                    std::scoped_lock lock(m_connection_mutex);
                    auto conn = shelter::make_ref<connection>();
                    conn->socket = std::move(socket);
                    m_connections.push_back(conn);
                } catch (asio::system_error const& e) {
                    if (m_is_running) fmt::print("{}\n", e.what());
                }
            }
        });

        m_receive = shelter::make_local<std::thread>([this] {
            while(m_is_running) {
                std::scoped_lock lock(m_connection_mutex);
                for (std::size_t i = 0; i < m_connections.size(); i++) {
                    try {
                        auto conn = m_connections[i];
                        conn->socket->async_read_some(asio::buffer(conn->read_buffer),
                            [c = conn]([[maybe_unused]]asio::error_code const& err, [[maybe_unused]]std::size_t bytes) {
                            // TODO: What are we reading
                        });
                    } catch (asio::system_error const&) { /* Ignore for now */ }
                }
            }
        });

        m_send = shelter::make_local<std::thread>([this] {
            while(m_is_running) {
                if (!m_unsent_messages.empty()) {
                    auto const& msg = m_unsent_messages.front();
                    sky::mcp_buffer buffer{};
                    sky::make_mcp_buffer(buffer, msg);
                    // TODO: Send to correct place
                    m_unsent_messages.pop();
                }
            }
        });

        for (std::size_t i = 0; i < m_graph.size(); i++) {
            create_hardware();
        }
    }

    auto create_hardware() -> void {
        if (!m_is_running || m_is_closing) return;
        auto hw = shelter::make_ref<hardware>();
        hw->id = m_id++;
        hw->graph = m_graph;
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
            m_id = 1;

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

    auto setGraph(std::vector<node> graph) {
        m_graph = graph;
    }

    auto is_running() -> bool { return m_is_running; }
    auto is_closing() -> bool { return m_is_closing; }

private:
    asio::io_context m_io{};
    shelter::local<std::thread>   m_thread{nullptr};
    shelter::local<std::thread>   m_close{nullptr};
    shelter::local<std::thread>   m_listen{nullptr};
    shelter::local<std::thread>   m_receive{nullptr};
    shelter::local<std::thread>   m_send{nullptr};
    shelter::ref<acceptor_t>      m_acceptor{nullptr};
    std::atomic<bool>             m_is_running{false};
    std::atomic<bool>             m_is_closing{false};
    std::vector<hardware_ref_t>   m_hardwares{};
    std::vector<node>             m_graph{};
    std::queue<sky::mcp>          m_unsent_messages{};
    std::uint32_t                 m_id{1};
    std::vector<connection_ref_t> m_connections{};
    std::mutex                    m_connection_mutex{};
};
} // namespace flicker


#endif  // SHELTER_FLICKER_HPP
