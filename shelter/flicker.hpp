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


#include "shelter/utility.hpp"
#include "asio.hpp"
#include "fmt/format.h"
#include "glm/vec2.hpp"
#include "sky.hpp"


namespace flicker {
struct hardware {
    std::uint32_t               id{};
    std::atomic<bool>           is_running{false};
    shelter::local<std::thread> thread{nullptr};

    auto loop() -> void {
        // fmt::print("{:#04x}: Hello, World!\n", id);
    }
};

using edges_t = std::array<std::uint32_t, 4>;
struct node {
    std::uint32_t id;
    edges_t       edges;  // N, E, S, W
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
                std::array<char, 256> buf;
                asio::error_code err;
                for (std::size_t i=0; i < m_sockets.size(); i++) {
                    try {
                        auto length = m_sockets[i]->read_some(asio::buffer(buf), err);
                        if (length == 0) {
                            continue;
                        }

                        sky::mcp message{};

                        message.type = (std::uint8_t) buf[0];
                        for (std::size_t j = 0; j < sizeof(sky::address_t); j++) {
                            message.source[j] = static_cast<std::uint8_t>(buf[j+1]);
                        }
                        for (std::size_t j = 0; j < sizeof(sky::address_t); j++) {
                            message.destination[j] = static_cast<std::uint8_t>(buf[j+4]);
                        }
                        for (std::size_t j = 0; j < sizeof(sky::payload_t); j++) {
                            message.payload[j] = static_cast<std::uint8_t>(buf[j + 7]);
                        }
                        message.crc = static_cast<std::uint8_t>(buf[22]);
                        m_unsent_messages.push(message);
                    }
                    catch (asio::system_error const&) { /* Ignore for now */ }
                  
                }
            }
        });

        m_send = shelter::make_local<std::thread>([this] {
            while(m_is_running) {
                if (!m_unsent_messages.empty()) {
                    auto message = m_unsent_messages.front();
                    std::uint32_t source = 0;
                    std::uint32_t destination = 0;

                    source = message.source[0];
                    source |= message.source[1] << 8;
                    source |= message.source[2] << 16;
                    
                    destination = message.destination[0];
                    destination |= message.destination[1] << 8;
                    destination |= message.destination[2] << 16;

                    auto socket = m_sockets[destination-1];

                    auto const& source_node = m_graph[source - 1];
                    bool found = false;
                    for (std::size_t i = 0; i < source_node.edges.size(); i++) {
                        if (source_node.edges[i] == destination) {
                            found = true;
                            break;
                        }
                    }

                    if (found) {
                        std::array<char, 256> buf{};
                        buf[0] = (char) message.type;
                        for (std::size_t j = 0; j < sizeof(sky::address_t); j++) {
                            buf[j + 1] = static_cast<char>(message.source[j]);
                        }
                        for (std::size_t j = 0; j < sizeof(sky::address_t); j++) {
                            buf[j + 4] = static_cast<char>(message.destination[j]);
                        }
                        for (std::size_t j = 0; j < sizeof(sky::payload_t); j++) {
                            buf[j + 7] = static_cast<char>(message.payload[j]);
                        }
                        buf[22] = static_cast<char>(message.crc);

                        socket->send(asio::buffer(buf));
                    }
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
    std::vector<node>           m_graph;
    std::queue<sky::mcp>        m_unsent_messages;
    std::uint32_t               m_id{1};
};
} // namespace flicker


#endif  // SHELTER_FLICKER_HPP
