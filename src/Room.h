#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "Mesh.h"
#include "Shader.h"

struct RoomObject {
    std::string name;
    glm::vec3 position;   // centre of the box
    glm::vec3 size;       // width, height, depth
    glm::vec3 color;
};

class Room {
public:
    Room();
    ~Room();

    void add(const std::string& name, glm::vec3 position, glm::vec3 size, glm::vec3 color);
    void draw(const Shader& shader) const;

private:
    void buildWalls();

    std::vector<RoomObject> objects;
    Mesh* cube;
};
