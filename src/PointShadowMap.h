#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

// Shadows for a point light (the desk lamp). A point light shines in every
// direction, so one flat depth texture is not enough: the scene is drawn six
// times into the faces of a cube map, one face per axis (+X, -X, +Y, -Y, +Z, -Z).
// Each texel stores the distance from the light to the closest surface.
class PointShadowMap {
public:
    explicit PointShadowMap(int size = 1024, float farPlane = 10.0f);
    ~PointShadowMap();

    // Starts drawing face 0..5 of the cube map; returns projection * view for that face.
    glm::mat4 beginFace(int face, const glm::vec3& lightPos);
    // Goes back to the normal window framebuffer.
    void end(int framebufferWidth, int framebufferHeight);

    void bindTexture(int unit) const;
    float getFarPlane() const { return farPlane; }

private:
    GLuint fbo = 0, depthCubemap = 0;
    int size;
    float farPlane;
};
