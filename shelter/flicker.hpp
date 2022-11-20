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

#include "asio.hpp"

namespace flicker {
class app {
using tcp_acceptor_t = asio::use_awaitable_t<>::as_default_on_t<asio::ip::tcp::acceptor>;
using tcp_socket_t   = asio::use_awaitable_t<>::as_default_on_t<asio::ip::tcp::socket>;

public:
    app(std::uint16_t port) : m_context(), m_port(port) {}
    ~app() { stop(); }

    auto start() -> void {
        if (m_is_running) return;
        try {
            m_thread = std::thread([this] {
                m_is_running = true;
                m_context.run();
                m_context.reset();
                m_is_running = false;
                fmt::print("flicker::app stopped\n");
            });
            asio::co_spawn(m_context, std::bind(&app::listener, this), asio::detached);
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
        tcp_acceptor_t acceptor(m_context, {asio::ip::tcp::v4(), m_port});
        fmt::print("flicker::app@{}:{}\n", acceptor.local_endpoint().address().to_string(), acceptor.local_endpoint().port());
        while (true) {
            auto socket = co_await acceptor.async_accept();
            asio::co_spawn(m_context, receive(std::move(socket)), asio::detached);
        }
    }

    auto receive(tcp_socket_t socket) -> asio::awaitable<void> {
        try {
            std::array<char, 1024> data{};
            while (true) {
                auto n = co_await socket.async_read_some(asio::buffer(data));
                co_await asio::async_write(socket, asio::buffer(data, n));
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
};
} // namespace flicker

#endif  // SHELTER_FLICKER_HPP
