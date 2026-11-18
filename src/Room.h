#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

struct RoomObject {
    std::string name;
    glm::vec3 position;   
    glm::vec3 size;       
    glm::vec3 color;
    const Texture* texture;   // nullptr = plain colour
    glm::vec3 texRepeat;      // how many times the texture repeats along x, y, z
};

class Room {
public:
    Room();
    ~Room();

    void add(
        const std::string& name,
        glm::vec3 position,
        glm::vec3 size,
        glm::vec3 color,
        const Texture* texture = nullptr,
        float tileSize = 1.0f   // metres covered by one copy of the texture; 0 = stretch once
    );

    // skipLamp leaves out the desk lamp itself: its bulb is inside the shade,
    // so in the lamp's own shadow pass the shade would hide everything
    void draw(const Shader& shader, bool skipLamp = false) const;

private:
    void buildWalls();
    void buildAidanaObjects(); 

    std::vector<RoomObject> objects;
    Mesh* cube;

    std::unique_ptr<Texture> woodFloor, wallPlaster, woodGrain, rugTexture;
};