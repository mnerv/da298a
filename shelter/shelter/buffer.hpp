/**
 * @file   buffer.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  buffer abstractions
 * @date   2022-11-08
 *
 * @copyright Copyright (c) 2022
 */
#ifndef SHELTER_BUFFER_HPP
#define SHELTER_BUFFER_HPP

#include <string>
#include <vector>

#include "common.hpp"
#include "graphics_context.hpp"

namespace shelter {
enum class data_type {
    boolean,
    i8,  u8,
    i16, u16,
    i32, u32, p32,
    i64, u64, p64,
    f16, f32, f64,

    // vector (f32, i32, f64)
    vec2,  vec3,  vec4,
    ivec2, ivec3, ivec4,
    dvec2, dvec3, dvec4,

    // matrix nxn (f32)
    mat2, mat3, mat4,
};

struct buffer_element {
    data_type     type;
    std::string   name;
    bool          normalized = false;
    std::uint32_t offset     = 0;

    // shader data types size in bytes
    // return size in bytes
    inline static auto type_size(data_type const& type) -> std::uint32_t {
        switch(type) {
            case data_type::i8:
            case data_type::u8:    return 1;

            case data_type::i16:
            case data_type::u16:
            case data_type::f16:   return 2;

            case data_type::i32:
            case data_type::u32:
            case data_type::p32:
            case data_type::f32:   return 4;
            case data_type::f64:   return 8;

            case data_type::ivec2:
            case data_type::vec2:  return 4 * 2;

            case data_type::ivec3:
            case data_type::vec3:  return 4 * 3;

            case data_type::ivec4:
            case data_type::vec4:  return 4 * 4;

            case data_type::dvec2: return 8 * 2;
            case data_type::dvec3: return 8 * 3;
            case data_type::dvec4: return 8 * 4;

            case data_type::mat2:  return 4 * 2 * 2;
            case data_type::mat3:  return 4 * 3 * 3;
            case data_type::mat4:  return 4 * 4 * 4;

            default: return 0;
        }
    }

    inline static auto component_count(data_type const& type) -> std::int32_t {
        switch(type) {
            case data_type::vec2:
            case data_type::ivec2:
            case data_type::dvec2: return 2;

            case data_type::vec3:
            case data_type::ivec3:
            case data_type::dvec3: return 3;

            case data_type::vec4:
            case data_type::ivec4:
            case data_type::dvec4: return 4;

            case data_type::mat2:  return 2 * 2;
            case data_type::mat3:  return 3 * 3;
            case data_type::mat4:  return 4 * 4;

            default: return 1;
        }
    }
};

class buffer_layout {
public:
    using elements_t = std::vector<buffer_element>;

public:
    buffer_layout(std::initializer_list<buffer_element> const& elements);
    ~buffer_layout() = default;

    auto stride() const -> std::size_t { return m_stride; };
    auto elements() const -> elements_t const& { return m_elements; }

    auto begin() -> elements_t::iterator { return std::begin(m_elements); }
    auto end()   -> elements_t::iterator { return std::end(m_elements); }
    auto begin() const -> elements_t::const_iterator { return std::begin(m_elements); }
    auto end()   const -> elements_t::const_iterator { return std::end(m_elements); }

private:
    auto compute_stride() const -> std::size_t;
    auto compute_offset() -> void;

private:
    std::size_t m_stride;
    elements_t  m_elements;
};

class vertex_buffer {
public:
    vertex_buffer(void const* data, std::uint32_t const& byte_size, buffer_layout const& layout);
    ~vertex_buffer();

    auto bind() const -> void;
    auto unbind() const -> void;

private:
    std::uint32_t m_buffer{};
    std::uint32_t m_array_buffer{};
    buffer_layout m_layout{};
};

class index_buffer {
public:
    index_buffer(void const* data, std::uint32_t const& byte_size, std::uint32_t const& size);
    ~index_buffer();

    auto bind() const -> void;
    auto unbind() const -> void;

    auto size() const -> std::int32_t { return m_size; }
    auto type() const -> std::uint32_t { return m_type; }

private:
    std::uint32_t m_buffer{};
    std::int32_t  m_size{};
    std::uint32_t m_type{};
};

auto make_vertex_buffer(graphics_context_ref_t context, void const* data, std::uint32_t const& byte_size, buffer_layout const& layout) -> vertex_buffer_ref_t;
auto make_index_buffer(graphics_context_ref_t context, void const* data, std::uint32_t const& byte_size, std::uint32_t const& count) -> index_buffer_ref_t;
} // namespace shelter

#endif  // SHELTER_BUFFER_HPP
