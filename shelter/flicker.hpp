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

#include <atomic>
#include <thread>
#include <vector>

#include "fmt/format.h"
#include "asio.hpp"

#include "shelter/utility.hpp"

namespace flicker {
using acceptor_t = asio::use_awaitable_t<>::as_default_on_t<asio::ip::tcp::acceptor>;
using socket_t   = asio::use_awaitable_t<>::as_default_on_t<asio::ip::tcp::socket>;
using buffer_t   = std::array<char, 2048>;

enum class compass : std::uint8_t {
    north,
    east ,
    south,
    west ,
};

struct message {
    compass heading;
    std::vector<char> body;
};

using trench_ref_t = shelter::ref<class trench>;
class trench {
public:
    trench(socket_t socket) : m_socket(std::move(socket)) { }

    auto send(buffer_t const& data) -> asio::awaitable<void> {
        m_data_out = data;
        co_await asio::async_write(m_socket, asio::buffer(data));
    }

    auto read() -> asio::awaitable<buffer_t> {
        co_await m_socket.async_read_some(asio::buffer(m_data_in));
        co_return m_data_in;
    }
    auto is_connected() const -> bool { return m_socket.is_open(); }

private:
    socket_t m_socket;
    buffer_t m_data_in{};
    buffer_t m_data_out{};
};

class client {
public:
    client(std::uint32_t id, trench_ref_t conn) : m_id(id), m_trench(std::move(conn)) {}

    auto loop() -> asio::awaitable<void> {
        throw std::runtime_error("");
        (void)m_id;
    }

private:
    std::uint32_t m_id;
    trench_ref_t  m_trench;
};

class app {
public:
    app(std::uint16_t port) : m_context(), m_port(port) {}
    ~app() { stop(); }

    auto start() -> void {
        if (m_is_running) return;
        try {
            asio::co_spawn(m_context, std::bind(&app::listener, this), asio::detached);
            m_thread = std::thread([this] {
                m_context.run();
                m_context.reset();
                m_is_running = false;
                fmt::print("flicker::app stopped\n");
            });
        } catch (asio::system_error const& e) {
            fmt::print("flicker::app {}\n", e.what());
            m_is_running = false;
        }
    }
    auto stop() -> void {
        if (!m_is_running) return;
        m_context.stop();
        if (m_thread.joinable()) m_thread.join();
    }
    auto is_running() const -> bool { return m_is_running; }

private:
    auto listener() -> asio::awaitable<void> {
        acceptor_t acceptor(m_context, {asio::ip::tcp::v4(), m_port});
        fmt::print("flicker::app@{}:{}\n", acceptor.local_endpoint().address().to_string(), acceptor.local_endpoint().port());
        m_is_running = true;
        while (true) {
            auto socket = co_await acceptor.async_accept();
            auto new_fusion = shelter::make_ref<trench>(std::move(socket));
            fmt::print("new trench\n");
            m_trenchs.push_back(new_fusion);
            asio::co_spawn(m_context, receive(std::move(new_fusion)), asio::detached);
        }
    }

    auto receive(trench_ref_t conn) -> asio::awaitable<void> {
        try {
            while (true) {
                auto const data = co_await conn->read();
                co_await conn->send(data);
            }
        } catch (asio::system_error const& e) {
            fmt::print("{}\n", e.what());
        }
    }

private:
    asio::io_context  m_context;
    std::uint16_t     m_port;
    std::atomic<bool> m_is_running{false};
    std::thread       m_thread{};
    std::vector<trench_ref_t> m_trenchs{};
};
} // namespace flicker

#endif  // SHELTER_FLICKER_HPP
