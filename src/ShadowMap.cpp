#include "ShadowMap.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

ShadowMap::ShadowMap(int size) : size(size) {
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, size, size, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    // LINEAR + compare mode: the GPU compares 4 texels and blends them,
    // which already softens the shadow edge a little
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    // everything outside the shadow map counts as lit
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float border[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    glDrawBuffer(GL_NONE);   // depth only, no colour
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Shadow map framebuffer is not complete\n";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ShadowMap::~ShadowMap() {
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &depthTexture);
}

void ShadowMap::begin(const glm::vec3& toLight) {
    // The sun is far away, so its rays are parallel: orthographic projection.
    // The box is just big enough for the 6 x 3 x 6 m room.
    glm::vec3 center(0.0f, 1.5f, 0.0f);
    glm::vec3 eye = center + glm::normalize(toLight) * 10.0f;
    glm::mat4 view = glm::lookAt(eye, center, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::ortho(-5.5f, 5.5f, -5.5f, 5.5f, 1.0f, 20.0f);
    lightSpace = projection * view;

    glViewport(0, 0, size, size);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Store the back faces instead of the front faces: the lit surfaces are then
    // never compared with themselves, which removes "shadow acne".
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
}

void ShadowMap::end(int framebufferWidth, int framebufferHeight) {
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
}

void ShadowMap::bindTexture(int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glActiveTexture(GL_TEXTURE0);
}
