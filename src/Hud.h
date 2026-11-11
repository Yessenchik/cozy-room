#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "Shader.h"

// 2D overlay drawn on top of the 3D scene: coloured rectangles and text.
// Coordinates are in window pixels, (0,0) is the top-left corner.
// Text uses stb_easy_font, which builds every letter out of small quads,
// so no font texture is needed.
class Hud {
public:
    Hud(const std::string& vertexPath, const std::string& fragmentPath);
    ~Hud();

    void begin(int windowWidth, int windowHeight);
    void rect(float x, float y, float w, float h, const glm::vec4& color);
    void text(float x, float y, const std::string& str, float scale, const glm::vec4& color);
    void textWithShadow(float x, float y, const std::string& str, float scale, const glm::vec4& color);
    void end();   // sends everything to the GPU and draws it in one call

    static float textWidth(const std::string& str, float scale);
    static float lineHeight(float scale) { return 12.0f * scale; }

private:
    struct HudVertex {
        glm::vec2 position;   // already in NDC
        glm::vec4 color;
    };

    void quad(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, const glm::vec4& color);
    glm::vec2 toNdc(float x, float y) const;

    Shader shader;
    GLuint vao = 0, vbo = 0;
    std::vector<HudVertex> vertices;
    float width = 1.0f, height = 1.0f;
};
