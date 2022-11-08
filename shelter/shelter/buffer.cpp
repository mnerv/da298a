/**
 * @file   buffer.cpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  buffer abstractions
 * @date   2022-11-08
 *
 * @copyright Copyright (c) 2022
 */
#include <stdexcept>
#include <numeric>
#include <algorithm>

#include "buffer.hpp"
#include "glad/glad.h"

namespace shelter {
auto make_vertex_buffer(graphics_context_ref_t context, void const* data, std::uint32_t const& byte_size, buffer_layout const& layout) -> vertex_buffer_ref_t {
    if (context == nullptr) throw std::runtime_error("shelter::make_vertex_buffer: context cannot be nullptr!");
    return make_ref<vertex_buffer>(data, byte_size, layout);
}

auto make_index_buffer(graphics_context_ref_t context, void const* data, std::uint32_t const& byte_size, std::uint32_t const& size) -> index_buffer_ref_t {
    if (context == nullptr) throw std::runtime_error("shelter::make_index_buffer: context cannot be nullptr!");
    return make_ref<index_buffer>(data, byte_size, size);
}

buffer_layout::buffer_layout(std::initializer_list<buffer_element> const& elements)
    : m_elements(elements) {
    m_stride = compute_stride();
    compute_offset();  // set the correct element data offset
}

auto buffer_layout::compute_stride() const -> std::size_t {
    return std::accumulate(std::begin(m_elements), std::end(m_elements), std::size_t(0),
    [](auto const& acc, auto const& b) {
        return acc + buffer_element::type_size(b.type);
    });
}

auto buffer_layout::compute_offset() -> void {
    std::uint32_t offset = 0;
    for (std::size_t i = 0; i < m_elements.size(); i++) {
        m_elements[i].offset = offset;
        offset += buffer_element::type_size(m_elements[i].type);
    }
}

vertex_buffer::vertex_buffer(void const* data, std::uint32_t const& byte_size, buffer_layout const& layout)
    : m_layout(layout) {
    // Create Vertex Array Buffer Object
    glGenVertexArrays(1, &m_array_buffer);
    glBindVertexArray(m_array_buffer);

    // Create Vertex Buffer Object
    glGenBuffers(1, &m_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
    glBufferData(GL_ARRAY_BUFFER, byte_size, data, GL_STATIC_DRAW);

    // Configure data layout
    auto const stride = m_layout.stride();
    std::uint32_t index = 0;
    std::for_each(std::begin(layout), std::end(layout), [&](buffer_element const& e) {
        auto const size   = buffer_element::component_count(e.type);
        auto const offset = e.offset;
        switch(e.type) {
            case data_type::f32:
            case data_type::vec2:
            case data_type::vec3:
            case data_type::vec4:
            case data_type::mat2:
            case data_type::mat3:
            case data_type::mat4:
                glVertexAttribPointer(index, size, GL_FLOAT, e.normalized ? GL_TRUE : GL_FALSE,
                                      GLsizei(stride), (void const*)std::size_t(offset));
                glEnableVertexAttribArray(index++);
                break;
            default: break;
        }
    });
}
vertex_buffer::~vertex_buffer() {
    glDeleteVertexArrays(1, &m_array_buffer);
    glDeleteBuffers(1, &m_buffer);
}

auto vertex_buffer::bind() const -> void {
    glBindVertexArray(m_array_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
}
auto vertex_buffer::unbind() const -> void {
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

index_buffer::index_buffer(void const* data, std::uint32_t const& byte_size, std::uint32_t const& size) : m_size(size) {
    glGenBuffers(1, &m_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, byte_size, data, GL_STATIC_DRAW);
    m_type = GL_UNSIGNED_INT;
}
index_buffer::~index_buffer() {
    glDeleteBuffers(1, &m_buffer);
}

auto index_buffer::bind() const -> void {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer);
}
auto index_buffer::unbind() const -> void {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
} // namespace shelter
