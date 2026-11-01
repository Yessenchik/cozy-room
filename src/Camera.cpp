#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

Camera::Camera(glm::vec3 target, float distance) : target(target), distance(distance) {}

// spherical coordinates (yaw, pitch, distance) -> point on a sphere around the target
glm::vec3 Camera::getPosition() const {
    float y = glm::radians(yaw);
    float p = glm::radians(pitch);
    glm::vec3 offset(std::cos(p) * std::cos(y), std::sin(p), std::cos(p) * std::sin(y));
    return target + offset * distance;
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(getPosition(), target, glm::vec3(0, 1, 0));
}

glm::mat4 Camera::getProjectionMatrix(float aspect) const {
    return glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);
}

void Camera::rotate(float yawDelta, float pitchDelta) {
    yaw += yawDelta;
    pitch = std::clamp(pitch + pitchDelta, 5.0f, 85.0f);
}

void Camera::zoom(float amount) {
    distance = std::clamp(distance - amount, 2.0f, 20.0f);
}
