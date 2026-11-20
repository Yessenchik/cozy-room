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
    const Texture* texture = nullptr;
    glm::vec3 texRepeat{ 1.0f };
};

struct SceneNode {
    glm::mat4 localTransform{ 1.0f };
    glm::vec3 size{ 1.0f };
    glm::vec3 color{ 1.0f };
    bool hasMesh = false;
    std::vector<SceneNode> children;
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
        float tileSize = 1.0f
    );

    void draw(const Shader& shader, bool skipLamp = false) const;
    void rotateLampArm(float degrees);
    glm::vec3 getLampLightPosition() const;
    void toggleLaptopScreen();

private:
    void buildWalls();
    void buildAidanaObjects();
    void buildWallArt();

    std::vector<RoomObject> objects;
    SceneNode lampRoot;
    float lampArmAngle = 0.0f;
    bool laptopScreenOn = false;
    Mesh* cube = nullptr;

    std::unique_ptr<Texture> woodFloor;
    std::unique_ptr<Texture> wallPlaster;
    std::unique_ptr<Texture> woodGrain;
    std::unique_ptr<Texture> rugTexture;
    std::unique_ptr<Texture> saturnArt;
};