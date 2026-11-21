#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "Shader.h"

class DustParticles {
public:
    DustParticles(
        const std::string& vertexPath,
        const std::string& fragmentPath,
        int particleCount = 220
    );

    ~DustParticles();

    DustParticles(const DustParticles&) = delete;
    DustParticles& operator=(const DustParticles&) = delete;

    void draw(
        const glm::vec3& windowOrigin,
        const glm::mat4& view,
        const glm::mat4& projection,
        float viewportHeight,
        float timeSeconds,
        float sunlightIntensity
    ) const;

private:
    struct Particle {
        glm::vec3 offset;
        float size;
        float phase;
    };

    Shader shader;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLsizei particleCount = 0;
    std::vector<Particle> particles;
};