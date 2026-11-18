#include "PointShadowMap.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

PointShadowMap::PointShadowMap(int size, float farPlane) : size(size), farPlane(farPlane) {
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (int face = 0; face < 6; ++face)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_DEPTH_COMPONENT24,
                     size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X, depthCubemap, 0);
    glDrawBuffer(GL_NONE);   // depth only, no colour
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Point shadow framebuffer is not complete\n";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

PointShadowMap::~PointShadowMap() {
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &depthCubemap);
}

glm::mat4 PointShadowMap::beginFace(int face, const glm::vec3& lightPos) {
    // Look direction and "up" for each cube face, in the order OpenGL expects them.
    // The up vectors follow the cube map convention, so lookups in the shader match.
    static const glm::vec3 dirs[6] = {
        { 1, 0, 0}, {-1, 0, 0}, {0,  1, 0}, {0, -1, 0}, {0, 0,  1}, {0, 0, -1}};
    static const glm::vec3 ups[6] = {
        {0, -1, 0}, {0, -1, 0}, {0,  0, 1}, {0, 0, -1}, {0, -1, 0}, {0, -1, 0}};

    // 90 degree field of view with a square image covers exactly one face
    glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.05f, farPlane);
    glm::mat4 view = glm::lookAt(lightPos, lightPos + dirs[face], ups[face]);

    glViewport(0, 0, size, size);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, depthCubemap, 0);
    glClear(GL_DEPTH_BUFFER_BIT);

    // back faces only, like the sun shadow map, to avoid shadow acne
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    return projection * view;
}

void PointShadowMap::end(int framebufferWidth, int framebufferHeight) {
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
}

void PointShadowMap::bindTexture(int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    glActiveTexture(GL_TEXTURE0);
}
