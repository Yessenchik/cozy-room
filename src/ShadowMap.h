#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

// Shadow mapping for the sun (and the moon at night).
// Pass 1: the scene is drawn from the light into a depth texture.
// Pass 2: basic.frag compares each pixel with that texture to see if it is in shadow.
class ShadowMap {
public:
    explicit ShadowMap(int size = 2048);
    ~ShadowMap();

    // Builds the light's view/projection and starts drawing into the depth texture.
    void begin(const glm::vec3& toLight);
    // Goes back to the normal window framebuffer.
    void end(int framebufferWidth, int framebufferHeight);

    void bindTexture(int unit) const;
    const glm::mat4& getLightSpaceMatrix() const { return lightSpace; }

private:
    GLuint fbo = 0, depthTexture = 0;
    int size;
    glm::mat4 lightSpace{1.0f};
};
