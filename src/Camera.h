#pragma once
#include <glm/glm.hpp>

class Camera {
public:
    Camera(glm::vec3 target, float distance);

    glm::vec3 getPosition() const;
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspect) const;

    void rotate(float yawDelta, float pitchDelta);
    void zoom(float amount);

private:
    glm::vec3 target;
    float distance;
    float yaw = 45.0f;
    float pitch = 30.0f;
    float fov = 45.0f;
};
