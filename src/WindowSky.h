#pragma once
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Shader.h"

// The procedural sky seen through the window (Aruzhan's window_sky shaders):
// a sky cube drawn behind the whole scene, and a thin pane in the window opening
// that shows the same sky from the camera's point of view.
// Moved out of main.cpp so that no source file is longer than ~500 lines.
class WindowSky {
public:
    WindowSky();
    ~WindowSky();

    // the sky cube, drawn before the room; depth is not written
    void drawBackground(const glm::mat4& view, const glm::mat4& projection,
                        const glm::vec3& cameraPos, float hour, float time) const;

    // the window pane, drawn after the room
    void drawPane(const glm::mat4& view, const glm::mat4& projection,
                  const glm::vec3& cameraPos, float hour, float time) const;

private:
    // points on the window where the sun and the moon are painted
    static glm::vec3 sunDirection(const glm::vec3& cameraPos);
    static glm::vec3 moonDirection(const glm::vec3& cameraPos);

    Shader skyShader;
    Shader paneShader;
    Mesh* skyCube = nullptr;
    Mesh* paneCube = nullptr;
    glm::mat4 paneModel{ 1.0f };
};
