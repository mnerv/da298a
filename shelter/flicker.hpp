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

constexpr std::size_t buffer_size = 256;
using edges_t = std::array<std::uint32_t, 4>;

struct node {
    std::uint32_t id;
    edges_t       edges;  // N, E, S, W
};

using acceptor_t   = asio::ip::tcp::acceptor;
using socket_t     = asio::ip::tcp::socket;
using socket_ref_t = shelter::ref<socket_t>;
using hardware_ref_t = shelter::ref<class hardware>;

using connection_buffer_t = std::array<char, buffer_size>;
using connection_ref_t    = shelter::ref<struct connection>;

struct connection {
    socket_ref_t        socket{nullptr};
    connection_buffer_t read_buffer{};
    connection_buffer_t send_buffer{};
};

class hardware {
public:
    hardware(std::uint32_t const& id) : m_id(id), m_graph({}), m_io(), m_socket(m_io) {
        m_thread = shelter::make_local<std::thread>([&] {
            m_io.run();
            asio::ip::tcp::resolver resolver(m_io);
            auto endpoints = resolver.resolve("localhost", "3000");
            asio::connect(m_socket, endpoints);
            fmt::print("shelter::hardware ID[{:#02d}] connected!\n", m_id);
        });
    }
    ~hardware() {
        m_io.stop();
        m_thread->join();
        fmt::print("shelter::hardware ID[{:#02d}] disconnected!\n", m_id);
    }

    auto set_graph(std::vector<node> graph) -> void { m_graph = std::move(graph); }

    auto loop() -> void {
        m_socket.async_read_some(asio::buffer(read_buffer),
            [&]([[maybe_unused]] asio::error_code const& error, [[maybe_unused]] std::size_t bytes_transferred) {
        });

        // TODO: Send out data
        m_socket.async_send(asio::buffer(send_buffer),
            [&]([[maybe_unused]] asio::error_code const& error, [[maybe_unused]] std::size_t bytes_transferred) {
        });
    }

private:
    std::uint32_t     m_id;
    std::vector<node> m_graph;
    asio::io_context  m_io;
    asio::ip::tcp::socket       m_socket;
    shelter::local<std::thread> m_thread;
    std::array<char, buffer_size> read_buffer{};
    std::array<char, buffer_size> send_buffer{};
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

        m_simulation = shelter::make_local<std::thread>([this] {
            while (m_is_running) {
                std::scoped_lock lock(m_hardware_mutex);
                for (auto const& hw : m_hardwares) {
                    hw->loop();
                }
            }
        });

        for (auto i = 1u; i < 16; ++i) create_hardware();
    }

    auto create_hardware() -> void {
        if (!m_is_running || m_is_closing) return;
        std::scoped_lock lock(m_hardware_mutex);
        auto hw = shelter::make_ref<hardware>(m_id++);
        hw->set_graph(m_graph);
        m_hardwares.push_back(hw);
    }

    auto stop() -> void {
        if (!m_is_running || m_is_closing) return;
        m_is_running = false;
        m_is_closing = true;

        m_close = shelter::make_local<std::thread>([&]{
            m_hardwares = {};
            m_id = 1;

            m_acceptor->close();
            m_listen->join();
            m_simulation->join();
            m_receive->join();
            m_send->join();
            m_io.stop();
            m_thread->join();
            m_is_closing = false;
            fmt::print("flicker::app stopped.\n");
        });
    }

    auto set_graph(std::vector<node> graph) {
        m_graph = std::move(graph);
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
    shelter::local<std::thread>   m_simulation{nullptr};

    shelter::ref<acceptor_t>      m_acceptor{nullptr};
    std::atomic<bool>             m_is_running{false};
    std::atomic<bool>             m_is_closing{false};
    std::vector<hardware_ref_t>   m_hardwares{};
    std::vector<node>             m_graph{};
    std::queue<sky::mcp>          m_unsent_messages{};
    std::uint32_t                 m_id{1};
    std::vector<connection_ref_t> m_connections{};
    std::mutex                    m_connection_mutex{};
    std::mutex                    m_hardware_mutex{};
};
} // namespace flicker


#endif  // SHELTER_FLICKER_HPP
