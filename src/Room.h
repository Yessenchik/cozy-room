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
    glm::vec3 emissive{ 0.0f };   // light the part gives off itself (the lamp shade when it is on)
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
    void setLampOn(bool on);   // the shade glows while the lamp is on
    void setTimeOfDay(float hour);
    void update(float deltaTime);

private:
    void buildWalls();
    void buildAidanaObjects();
    void buildWallArt();
    void buildCat();

    std::vector<RoomObject> objects;
    SceneNode lampRoot;
    SceneNode catRoot;
    float lampArmAngle = 0.0f;
    float timeOfDay = 12.0f;
    int catActivity = -1;
    float catMoveElapsed = 0.0f;
    bool catMoving = false;
    bool laptopScreenOn = false;
    glm::vec3 catPosition{ -1.9f, 0.60f, -1.0f };
    glm::vec3 catMoveStart{ -1.9f, 0.60f, -1.0f };
    glm::vec3 catMoveTarget{ -1.9f, 0.60f, -1.0f };
    Mesh* cube = nullptr;

    std::unique_ptr<Texture> woodFloor;
    std::unique_ptr<Texture> wallPlaster;
    std::unique_ptr<Texture> woodGrain;
    std::unique_ptr<Texture> rugTexture;
    std::unique_ptr<Texture> saturnArt;
};