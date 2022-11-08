/**
 * @file   shader.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  shader abstractions
 * @date   2022-11-08
 *
 * @copyright Copyright (c) 2022
 */
#include "shader.hpp"
#include <stdexcept>

#include "fmt/format.h"
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"

namespace shelter {
auto make_shader(graphics_context_ref_t context) -> shader_ref_t {
    if (context == nullptr) throw std::runtime_error("shelter::make_shader: context cannot be nullptr!");
    return make_ref<shader>();
}

static auto DEFAULT_VERTEX_SHADER = R"(#version 410 core
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_uv;

out vec4 io_color;
out vec2 io_uv;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
    io_color = a_color;
    io_uv    = a_uv;

    gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0f);
}
)";

static auto DEFAULT_FRAGMENT_SHADER = R"(#version 410 core
layout(location = 0) out vec4 color;

in vec4 io_color;
in vec2 io_uv;

uniform vec4 u_color;

void main() {
    color = u_color;
}
)";

shader::shader() {
    auto vs = compile(GL_VERTEX_SHADER, DEFAULT_VERTEX_SHADER);
    auto fs = compile(GL_FRAGMENT_SHADER, DEFAULT_FRAGMENT_SHADER);
    m_id = link(vs, fs);
}
shader::~shader() {
    glDeleteProgram(m_id);
}

auto shader::bind() const -> void {
    glUseProgram(m_id);
}
auto shader::unbind() const -> void {
    glUseProgram(0);
}

auto shader::upload(std::string const& name, std::uint32_t const& value) -> void {
    glUniform1ui(uniform_location(name), value);
}
auto shader::upload(std::string const& name, std::int32_t const& value) -> void {
    glUniform1i(uniform_location(name), value);
}
auto shader::upload(std::string const& name, float const& value) -> void {
    glUniform1f(uniform_location(name), value);
}
auto shader::upload(std::string const& name, std::int32_t const& count, float const* value) -> void {
    glUniform1fv(uniform_location(name), count, value);
}

auto shader::upload(std::string const& name, glm::vec2 const& value) -> void {
    glUniform2fv(uniform_location(name), 1, glm::value_ptr(value));
}
auto shader::upload(std::string const& name, glm::vec3 const& value) -> void {
    glUniform3fv(uniform_location(name), 1, glm::value_ptr(value));
}
auto shader::upload(std::string const& name, glm::vec4 const& value) -> void {
    glUniform4fv(uniform_location(name), 1, glm::value_ptr(value));
}

auto shader::upload(std::string const& name, glm::mat2 const& value, bool const& transpose) -> void {
    glUniformMatrix2fv(uniform_location(name), 1, (transpose ? GL_TRUE : GL_FALSE), glm::value_ptr(value));
}
auto shader::upload(std::string const& name, glm::mat3 const& value, bool const& transpose) -> void {
    glUniformMatrix3fv(uniform_location(name), 1, (transpose ? GL_TRUE : GL_FALSE), glm::value_ptr(value));
}
auto shader::upload(std::string const& name, glm::mat4 const& value, bool const& transpose) -> void {
    glUniformMatrix4fv(uniform_location(name), 1, (transpose ? GL_TRUE : GL_FALSE), glm::value_ptr(value));
}

auto shader::uniform_location(std::string const& name) const -> std::int32_t {
    return glGetUniformLocation(m_id, name.c_str());
}

auto shader::compile(std::uint32_t const& type, char const* source) -> std::uint32_t {
    std::uint32_t shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    std::int32_t is_success;
    constexpr auto LOG_SIZE = 512;
    static char info_log[LOG_SIZE];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &is_success);
    if (!is_success) {
        glGetShaderInfoLog(shader, LOG_SIZE, nullptr, info_log);
        auto const err = fmt::format("shelter::shader compile error: {}_SHADER: {}",
            type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT",
            info_log);
        throw std::runtime_error(err);
    }

    return shader;
}
auto shader::link(std::uint32_t const& vertex_shader, std::uint32_t const& fragment_shader) -> std::uint32_t {
    std::uint32_t program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    std::int32_t is_success;
    constexpr auto LOG_SIZE = 512;
    static char info_log[LOG_SIZE];
    glGetProgramiv(program, GL_LINK_STATUS, &is_success);
    if (!is_success) {
        glGetProgramInfoLog(program, LOG_SIZE, nullptr, info_log);
        auto const err = fmt::format("shelter::shader link error: {:s}", info_log);
        throw std::runtime_error(err);
    }

    glUseProgram(program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return program;
}
} // namespace shelter

