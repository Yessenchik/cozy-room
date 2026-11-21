#include "WindowSky.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

namespace {

// the window opening in the back wall
const glm::vec3 PANE_CENTER(0.5f, 1.64f, -3.115f);
const glm::vec3 PANE_SIZE(1.91f, 1.18f, 0.01f);

// a point on the pane, given as 0..1 across and up the window
glm::vec3 pointOnPane(float u, float v) {
    return glm::vec3(PANE_CENTER.x + (u - 0.5f) * PANE_SIZE.x,
                     PANE_CENTER.y + (v - 0.5f) * PANE_SIZE.y,
                     PANE_CENTER.z);
}

}  // namespace

WindowSky::WindowSky()
    : skyShader(std::string(PROJECT_DIR) + "shaders/window_sky.vert",
                std::string(PROJECT_DIR) + "shaders/window_sky.frag"),
      paneShader(std::string(PROJECT_DIR) + "shaders/window_plane.vert",
                 std::string(PROJECT_DIR) + "shaders/window_sky.frag") {
    skyCube = Mesh::createCube();
    paneCube = Mesh::createCube();
    paneModel = glm::scale(glm::translate(glm::mat4(1.0f), PANE_CENTER), PANE_SIZE);
}

WindowSky::~WindowSky() {
    delete paneCube;
    delete skyCube;
}

glm::vec3 WindowSky::sunDirection(const glm::vec3& cameraPos) {
    return glm::normalize(pointOnPane(0.76f, 0.70f) - cameraPos);
}

glm::vec3 WindowSky::moonDirection(const glm::vec3& cameraPos) {
    return glm::normalize(pointOnPane(0.78f, 0.72f) - cameraPos);
}

void WindowSky::drawBackground(const glm::mat4& view, const glm::mat4& projection,
                               const glm::vec3& cameraPos, float hour, float time) const {
    const GLboolean cullingWasEnabled = glIsEnabled(GL_CULL_FACE);
    GLint previousCullMode = GL_BACK;
    glGetIntegerv(GL_CULL_FACE_MODE, &previousCullMode);

    // we are inside the cube, so its front faces are culled; the sky sits at
    // the far plane (xyww in the shader) and never hides anything
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    skyShader.use();
    skyShader.setMat4("uView", glm::mat4(glm::mat3(view)));   // no translation
    skyShader.setMat4("uProjection", projection);
    skyShader.setFloat("uHour", hour);
    skyShader.setFloat("uTime", time);
    skyShader.setVec3("uSunDirection", sunDirection(cameraPos));
    skyShader.setVec3("uMoonDirection", moonDirection(cameraPos));
    skyCube->draw();

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glCullFace(previousCullMode);
    if (!cullingWasEnabled)
        glDisable(GL_CULL_FACE);
}

void WindowSky::drawPane(const glm::mat4& view, const glm::mat4& projection,
                         const glm::vec3& cameraPos, float hour, float time) const {
    paneShader.use();
    paneShader.setMat4("uModel", paneModel);
    paneShader.setMat4("uView", view);
    paneShader.setMat4("uProjection", projection);
    paneShader.setVec3("uCameraPos", cameraPos);
    paneShader.setVec3("uSunDirection", sunDirection(cameraPos));
    paneShader.setVec3("uMoonDirection", moonDirection(cameraPos));
    paneShader.setFloat("uHour", hour);
    paneShader.setFloat("uTime", time);
    paneCube->draw();
}
