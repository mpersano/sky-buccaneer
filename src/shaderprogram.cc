#include "shaderprogram.h"

#include "panic.h"

#include <array>
#include <fstream>

#include <glm/gtc/type_ptr.hpp>

namespace GL {

namespace {

std::vector<char> readFile(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
        panic("failed to open %s\n", std::string(path).c_str());

    auto *buf = file.rdbuf();

    const std::size_t size = buf->pubseekoff(0, file.end, file.in);
    buf->pubseekpos(0, file.in);

    std::vector<char> data(size + 1);
    buf->sgetn(data.data(), size);
    data[size] = 0;

    file.close();

    return data;
}

} // namespace

ShaderProgram::ShaderProgram()
    : m_id(glCreateProgram())
{
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(m_id);
}

void ShaderProgram::addShader(GLenum type, const std::string &filename)
{
    const auto source = readFile(filename);

    const auto shader_id = glCreateShader(type);

    const auto sourcePtr = source.data();
    glShaderSource(shader_id, 1, &sourcePtr, nullptr);
    glCompileShader(shader_id);

    int status;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
    if (!status) {
        std::array<GLchar, 64 * 1024> buf;
        GLsizei length;
        glGetShaderInfoLog(shader_id, buf.size() - 1, &length, buf.data());
        panic("failed to compile shader %s:\n%.*s", std::string(filename).c_str(), length, buf.data());
    }

    glAttachShader(m_id, shader_id);
}

void ShaderProgram::link()
{
    glLinkProgram(m_id);

    int status;
    glGetProgramiv(m_id, GL_LINK_STATUS, &status);
    if (!status)
        panic("failed to link shader program\n");
}

void ShaderProgram::bind() const
{
    glUseProgram(m_id);
}

int ShaderProgram::uniformLocation(std::string_view name) const
{
    return glGetUniformLocation(m_id, name.data());
}

void ShaderProgram::setUniform(int location, int value) const
{
    glUniform1i(location, value);
}

void ShaderProgram::setUniform(int location, float value) const
{
    glUniform1f(location, value);
}

void ShaderProgram::setUniform(int location, const glm::vec2 &value) const
{
    glUniform2fv(location, 1, glm::value_ptr(value));
}

void ShaderProgram::setUniform(int location, const glm::vec3 &value) const
{
    glUniform3fv(location, 1, glm::value_ptr(value));
}

void ShaderProgram::setUniform(int location, const glm::vec4 &value) const
{
    glUniform4fv(location, 1, glm::value_ptr(value));
}

void ShaderProgram::setUniform(int location, const std::vector<float> &value) const
{
    glUniform1fv(location, value.size(), value.data());
}

void ShaderProgram::setUniform(int location, const std::vector<glm::vec2> &value) const
{
    glUniform2fv(location, value.size(), reinterpret_cast<const float *>(value.data()));
}

void ShaderProgram::setUniform(int location, const std::vector<glm::vec3> &value) const
{
    glUniform3fv(location, value.size(), reinterpret_cast<const float *>(value.data()));
}

void ShaderProgram::setUniform(int location, const std::vector<glm::vec4> &value) const
{
    glUniform4fv(location, value.size(), reinterpret_cast<const float *>(value.data()));
}

void ShaderProgram::setUniform(int location, const glm::mat3 &value) const
{
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderProgram::setUniform(int location, const glm::mat4 &value) const
{
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

} // namespace GL
