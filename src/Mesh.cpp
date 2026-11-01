#include "Mesh.h"
#include <cstddef>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    : indexCount((GLsizei)indices.size()) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glBindVertexArray(0);
}

Mesh::~Mesh() {
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
}

void Mesh::draw() const {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
}

// 24 vertices instead of 8: each face needs its own normal
Mesh* Mesh::createCube() {
    std::vector<Vertex> v = {
        {{-0.5f, -0.5f,  0.5f}, { 0,  0,  1}}, {{ 0.5f, -0.5f,  0.5f}, { 0,  0,  1}},
        {{ 0.5f,  0.5f,  0.5f}, { 0,  0,  1}}, {{-0.5f,  0.5f,  0.5f}, { 0,  0,  1}},

        {{ 0.5f, -0.5f, -0.5f}, { 0,  0, -1}}, {{-0.5f, -0.5f, -0.5f}, { 0,  0, -1}},
        {{-0.5f,  0.5f, -0.5f}, { 0,  0, -1}}, {{ 0.5f,  0.5f, -0.5f}, { 0,  0, -1}},

        {{ 0.5f, -0.5f,  0.5f}, { 1,  0,  0}}, {{ 0.5f, -0.5f, -0.5f}, { 1,  0,  0}},
        {{ 0.5f,  0.5f, -0.5f}, { 1,  0,  0}}, {{ 0.5f,  0.5f,  0.5f}, { 1,  0,  0}},

        {{-0.5f, -0.5f, -0.5f}, {-1,  0,  0}}, {{-0.5f, -0.5f,  0.5f}, {-1,  0,  0}},
        {{-0.5f,  0.5f,  0.5f}, {-1,  0,  0}}, {{-0.5f,  0.5f, -0.5f}, {-1,  0,  0}},

        {{-0.5f,  0.5f,  0.5f}, { 0,  1,  0}}, {{ 0.5f,  0.5f,  0.5f}, { 0,  1,  0}},
        {{ 0.5f,  0.5f, -0.5f}, { 0,  1,  0}}, {{-0.5f,  0.5f, -0.5f}, { 0,  1,  0}},

        {{-0.5f, -0.5f, -0.5f}, { 0, -1,  0}}, {{ 0.5f, -0.5f, -0.5f}, { 0, -1,  0}},
        {{ 0.5f, -0.5f,  0.5f}, { 0, -1,  0}}, {{-0.5f, -0.5f,  0.5f}, { 0, -1,  0}},
    };

    std::vector<unsigned int> indices;
    for (unsigned int face = 0; face < 6; ++face) {
        unsigned int b = face * 4;
        indices.insert(indices.end(), {b, b + 1, b + 2, b, b + 2, b + 3});
    }
    return new Mesh(v, indices);
}
