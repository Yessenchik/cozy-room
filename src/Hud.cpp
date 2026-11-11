#include "Hud.h"
#include <cstddef>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include <stb_easy_font.h>
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

Hud::Hud(const std::string& vertexPath, const std::string& fragmentPath)
    : shader(vertexPath, fragmentPath) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(HudVertex), (void*)offsetof(HudVertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(HudVertex), (void*)offsetof(HudVertex, color));
    glBindVertexArray(0);
}

Hud::~Hud() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void Hud::begin(int windowWidth, int windowHeight) {
    width = windowWidth > 0 ? (float)windowWidth : 1.0f;
    height = windowHeight > 0 ? (float)windowHeight : 1.0f;
    vertices.clear();
}

glm::vec2 Hud::toNdc(float x, float y) const {
    return glm::vec2(x / width * 2.0f - 1.0f, 1.0f - y / height * 2.0f);
}

// a quad is two triangles, because core profile has no GL_QUADS
void Hud::quad(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, const glm::vec4& color) {
    vertices.push_back({a, color});
    vertices.push_back({b, color});
    vertices.push_back({c, color});
    vertices.push_back({a, color});
    vertices.push_back({c, color});
    vertices.push_back({d, color});
}

void Hud::rect(float x, float y, float w, float h, const glm::vec4& color) {
    quad(toNdc(x, y), toNdc(x + w, y), toNdc(x + w, y + h), toNdc(x, y + h), color);
}

void Hud::text(float x, float y, const std::string& str, float scale, const glm::vec4& color) {
    // stb_easy_font writes 4 vertices per quad, 16 bytes each: x, y, z (floats) + RGBA (bytes)
    static char buffer[99999];
    unsigned char unused[4] = {255, 255, 255, 255};
    int quads = stb_easy_font_print(0.0f, 0.0f, const_cast<char*>(str.c_str()), unused,
                                    buffer, sizeof(buffer));

    for (int q = 0; q < quads; ++q) {
        glm::vec2 corner[4];
        for (int i = 0; i < 4; ++i) {
            const float* v = reinterpret_cast<const float*>(buffer + (q * 4 + i) * 16);
            corner[i] = toNdc(x + v[0] * scale, y + v[1] * scale);
        }
        quad(corner[0], corner[1], corner[2], corner[3], color);
    }
}

void Hud::textWithShadow(float x, float y, const std::string& str, float scale, const glm::vec4& color) {
    text(x + scale, y + scale, str, scale, glm::vec4(0.0f, 0.0f, 0.0f, 0.7f * color.a));
    text(x, y, str, scale, color);
}

float Hud::textWidth(const std::string& str, float scale) {
    return stb_easy_font_width(const_cast<char*>(str.c_str())) * scale;
}

void Hud::end() {
    if (vertices.empty())
        return;

    // the overlay is flat and see-through: no depth test, alpha blending on
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader.use();
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(HudVertex), vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
