/**
 * @file   queue.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  Simple fixed size stack based queue implementation
 * @date   2022-12-01
 * @copyright Copyright (c) 2022
 */
#ifndef SKY_QUEUE_HPP
#define SKY_QUEUE_HPP

namespace sky {
template <typename T, size_t SIZE>
class queue {
public:
    queue() = default;
    queue(queue const&) = delete;
    queue& operator=(queue const&) = delete;
    queue(queue&&) = delete;

    auto enq(T const& data) -> void {
        m_buffer[m_tail] = data;
        inc(m_tail, SIZE);
        if (m_size == SIZE) inc(m_head, SIZE);
        else ++m_size;
    }

    [[nodiscard]] auto deq() noexcept -> T {
        if (m_head == m_tail) return m_buffer[m_head];
        auto const& value = m_buffer[m_head];
        inc(m_head, SIZE);
        --m_size;
        return value;
    }

    [[nodiscard]] auto empty() const noexcept -> bool {
        return m_size == 0;
    }

    [[nodiscard]] auto peek(size_t i) const -> T {
        return m_buffer[i];
    }

    [[nodiscard]] auto size() const noexcept -> size_t {
        return m_size;
    }
    [[nodiscard]] auto capacity() const noexcept -> size_t {
        return SIZE;
    }

private:
    static auto inc(size_t& value, size_t const& size) noexcept -> void {
        value = (value + 1) % size;
    }
    [[nodiscard]] auto next(size_t const& value) const noexcept -> size_t {
        return (value + 1) % SIZE;
    }

private:
    T m_buffer[SIZE]{};
    size_t m_head = 0;
    size_t m_tail = 0;
    size_t m_size = 0;
};
}

#endif  // !SKY_QUEUE_HPP
