#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

static std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Cannot open shader file: " << path << "\n";
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static GLuint compileShader(GLenum type, const std::string& source, const std::string& path) {
    GLuint shader = glCreateShader(type);
    const char* code = source.c_str();
    glShaderSource(shader, 1, &code, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader compile error in " << path << ":\n" << log << "\n";
    }
    return shader;
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    GLuint vertex = compileShader(GL_VERTEX_SHADER, readFile(vertexPath), vertexPath);
    GLuint fragment = compileShader(GL_FRAGMENT_SHADER, readFile(fragmentPath), fragmentPath);

    id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);

    GLint success;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(id, sizeof(log), nullptr, log);
        std::cerr << "Shader link error:\n" << log << "\n";
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader() {
    glDeleteProgram(id);
}

void Shader::use() const {
    glUseProgram(id);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setMat3(const std::string& name, const glm::mat3& value) const {
    glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
