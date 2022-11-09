/**
 * @file   shader.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  shader abstractions
 * @date   2022-11-08
 *
 * @copyright Copyright (c) 2022
 */
#ifndef SHELTER_SHADER_HPP
#define SHELTER_SHADER_HPP

#include <string>
#include <string_view>
#include <filesystem>

#include "common.hpp"
#include "graphics_context.hpp"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat2x2.hpp"
#include "glm/mat3x3.hpp"
#include "glm/mat4x4.hpp"

namespace shelter {
auto make_shader(graphics_context_ref_t context, std::string const& vs_source, std::string const& fs_source) -> shader_ref_t;

class shader {
public:
    shader(std::string const& vs_source, std::string const& fs_source);
    ~shader();

    auto bind() const -> void;
    auto unbind() const -> void;

public:
    auto upload(std::string const& name, std::uint32_t const& value) -> void;
    auto upload(std::string const& name, std::int32_t const& value) -> void;
    auto upload(std::string const& name, float const& value) -> void;
    auto upload(std::string const& name, std::int32_t const& count, float const* value) -> void;

    auto upload(std::string const& name, glm::vec2 const& value) -> void;
    auto upload(std::string const& name, glm::vec3 const& value) -> void;
    auto upload(std::string const& name, glm::vec4 const& value) -> void;

    auto upload(std::string const& name, glm::mat2 const& value, bool const& transpose = false) -> void;
    auto upload(std::string const& name, glm::mat3 const& value, bool const& transpose = false) -> void;
    auto upload(std::string const& name, glm::mat4 const& value, bool const& transpose = false) -> void;

private:
    [[nodiscard]] auto uniform_location(std::string const& name) const -> std::int32_t;

private:
    [[nodiscard]]static auto compile(std::uint32_t const& type, char const* source) -> std::uint32_t;
    [[nodiscard]]static auto link(std::uint32_t const& vertex_shader, std::uint32_t const& fragment_shader) -> std::uint32_t;

private:
    std::uint32_t m_id;
};
} // namespace shelter

#endif  // SHELTER_SHADER_HPP
