#include "DustParticles.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <random>

DustParticles::DustParticles(
    const std::string& vertexPath,
    const std::string& fragmentPath,
    int requestedParticleCount
)
    : shader(vertexPath, fragmentPath) {
    const int count = std::max(requestedParticleCount, 0);

    std::mt19937 generator(std::random_device{}());
    std::uniform_real_distribution<float> depthDistribution(0.03f, 1.75f);
    std::uniform_real_distribution<float> angleDistribution(0.0f, 6.2831853f);
    std::uniform_real_distribution<float> spreadDistribution(0.045f, 0.18f);
    std::uniform_real_distribution<float> sizeDistribution(0.010f, 0.026f);
    std::uniform_real_distribution<float> phaseDistribution(0.0f, 6.2831853f);

    particles.reserve(static_cast<std::size_t>(count));

    for (int i = 0; i < count; ++i) {
        const float depth = depthDistribution(generator);
        const float angle = angleDistribution(generator);
        const float radius = 0.018f + depth * spreadDistribution(generator);

        Particle particle;
        particle.offset = {
            std::cos(angle) * radius,
            -0.78f * depth + std::sin(angle) * radius * 0.45f,
            0.60f * depth + std::sin(angle) * radius * 0.35f
        };
        particle.size = sizeDistribution(generator);
        particle.phase = phaseDistribution(generator);

        particles.push_back(particle);
    }

    particleCount = static_cast<GLsizei>(particles.size());

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(particles.size() * sizeof(Particle)),
        particles.empty() ? nullptr : particles.data(),
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Particle),
        reinterpret_cast<void*>(offsetof(Particle, offset))
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        1,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Particle),
        reinterpret_cast<void*>(offsetof(Particle, size))
    );

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        1,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Particle),
        reinterpret_cast<void*>(offsetof(Particle, phase))
    );

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

DustParticles::~DustParticles() {
    if (vbo != 0) {
        glDeleteBuffers(1, &vbo);
    }

    if (vao != 0) {
        glDeleteVertexArrays(1, &vao);
    }
}

void DustParticles::draw(
    const glm::vec3& windowOrigin,
    const glm::mat4& view,
    const glm::mat4& projection,
    float viewportHeight,
    float timeSeconds,
    float sunlightIntensity
) const {
    if (particleCount == 0 ||
        viewportHeight <= 0.0f ||
        sunlightIntensity <= 0.01f) {
        return;
    }

    const GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);
    const GLboolean pointSizeWasEnabled =
        glIsEnabled(GL_PROGRAM_POINT_SIZE);

    GLboolean previousDepthMask = GL_TRUE;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &previousDepthMask);

    GLint previousSrcRgb = GL_ONE;
    GLint previousDstRgb = GL_ZERO;
    GLint previousSrcAlpha = GL_ONE;
    GLint previousDstAlpha = GL_ZERO;
    GLint previousProgram = 0;
    GLint previousVao = 0;

    glGetIntegerv(GL_BLEND_SRC_RGB, &previousSrcRgb);
    glGetIntegerv(GL_BLEND_DST_RGB, &previousDstRgb);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &previousSrcAlpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &previousDstAlpha);
    glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVao);

    glEnable(GL_BLEND);
    glBlendFuncSeparate(
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA,
        GL_ONE,
        GL_ONE_MINUS_SRC_ALPHA
    );
    glDepthMask(GL_FALSE);
    glEnable(GL_PROGRAM_POINT_SIZE);

    shader.use();
    shader.setMat4("uView", view);
    shader.setMat4("uProjection", projection);
    shader.setVec3("uWindowOrigin", windowOrigin);
    shader.setFloat("uTime", timeSeconds);
    shader.setFloat("uViewportHeight", viewportHeight);
    shader.setFloat("uSunlightIntensity", sunlightIntensity);

    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, particleCount);

    glBindVertexArray(static_cast<GLuint>(previousVao));
    glUseProgram(static_cast<GLuint>(previousProgram));

    glDepthMask(previousDepthMask);
    glBlendFuncSeparate(
        previousSrcRgb,
        previousDstRgb,
        previousSrcAlpha,
        previousDstAlpha
    );

    if (!blendWasEnabled) {
        glDisable(GL_BLEND);
    }

    if (!pointSizeWasEnabled) {
        glDisable(GL_PROGRAM_POINT_SIZE);
    }
}