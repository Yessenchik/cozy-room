#include "Room.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

const glm::vec3 FLOOR_COLOR(1.0f, 1.0f, 1.0f);
const glm::vec3 WALL_COLOR(0.56f, 0.70f, 0.90f);
const glm::vec3 FRAME_COLOR(0.95f, 0.95f, 0.92f);

const int COLOR_TEXTURE_UNIT = 0;

namespace {
    void drawNode(
        const SceneNode& node,
        const glm::mat4& parentTransform,
        const Shader& shader,
        const Mesh* cube
    ) {
        const glm::mat4 worldTransform = parentTransform * node.localTransform;

        if (node.hasMesh) {
            const glm::mat4 model = glm::scale(worldTransform, node.size);

            shader.setMat4("uModel", model);
            shader.setMat3(
                "uNormalMatrix",
                glm::mat3(glm::transpose(glm::inverse(model)))
            );
            shader.setVec3("uColor", node.color);
            shader.setVec3("uEmissive", node.emissive);
            shader.setInt("uUseTexture", 0);
            cube->draw();
        }

        for (const SceneNode& child : node.children) {
            drawNode(child, worldTransform, shader, cube);
        }
    }
}

Room::Room() {
    cube = Mesh::createCube();

    const std::string dir = std::string(PROJECT_DIR) + "textures/";
    woodFloor = std::make_unique<Texture>(dir + "wood_floor.png");
    wallPlaster = std::make_unique<Texture>(dir + "wall_plaster.png");
    woodGrain = std::make_unique<Texture>(dir + "wood_grain.png");
    rugTexture = std::make_unique<Texture>(dir + "rug.png");
    saturnArt = std::make_unique<Texture>(dir + "art_saturn.png");

    buildWalls();
    buildAidanaObjects();
    buildWallArt();

    add("desk top", { 0.5f, 0.82f, -2.35f }, { 1.8f, 0.12f, 1.0f }, { 0.48f, 0.27f, 0.13f });
    add("desk leg FL", { -0.30f, 0.40f, -2.75f }, { 0.10f, 0.8f, 0.10f }, { 0.30f, 0.16f, 0.08f });
    add("desk leg FR", { 1.30f, 0.40f, -2.75f }, { 0.10f, 0.8f, 0.10f }, { 0.30f, 0.16f, 0.08f });
    add("desk leg BL", { -0.30f, 0.40f, -1.95f }, { 0.10f, 0.8f, 0.10f }, { 0.30f, 0.16f, 0.08f });
    add("desk leg BR", { 1.30f, 0.40f, -1.95f }, { 0.10f, 0.8f, 0.10f }, { 0.30f, 0.16f, 0.08f });

    add("chair seat", { 0.5f, 0.48f, -1.35f }, { 0.65f, 0.12f, 0.65f }, { 0.18f, 0.30f, 0.38f });
    add("chair back", { 0.5f, 0.95f, -1.02f }, { 0.65f, 0.85f, 0.10f }, { 0.18f, 0.30f, 0.38f });
    add("chair leg FL", { 0.23f, 0.23f, -1.08f }, { 0.08f, 0.46f, 0.08f }, { 0.12f, 0.12f, 0.13f });
    add("chair leg FR", { 0.77f, 0.23f, -1.08f }, { 0.08f, 0.46f, 0.08f }, { 0.12f, 0.12f, 0.13f });
    add("chair leg BL", { 0.23f, 0.23f, -1.62f }, { 0.08f, 0.46f, 0.08f }, { 0.12f, 0.12f, 0.13f });
    add("chair leg BR", { 0.77f, 0.23f, -1.62f }, { 0.08f, 0.46f, 0.08f }, { 0.12f, 0.12f, 0.13f });

    add("laptop base", { 0.35f, 0.905f, -2.35f }, { 0.62f, 0.035f, 0.42f }, { 0.12f, 0.14f, 0.17f });
    add("laptop keyboard", { 0.35f, 0.927f, -2.37f }, { 0.48f, 0.012f, 0.23f }, { 0.24f, 0.27f, 0.30f });
    add("laptop touchpad", { 0.35f, 0.928f, -2.22f }, { 0.16f, 0.012f, 0.09f }, { 0.35f, 0.37f, 0.39f });
    add("laptop screen frame", { 0.35f, 1.11f, -2.53f }, { 0.58f, 0.38f, 0.04f }, { 0.035f, 0.04f, 0.05f });
    add("laptop display", { 0.35f, 1.11f, -2.505f }, { 0.52f, 0.32f, 0.008f }, { 0.01f, 0.015f, 0.025f });

    lampRoot.localTransform = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(1.18f, 0.91f, -2.35f)
    );
    lampRoot.hasMesh = true;
    lampRoot.size = { 0.38f, 0.06f, 0.30f };
    lampRoot.color = { 0.10f, 0.12f, 0.15f };

    SceneNode stem;
    stem.localTransform = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(0.0f, 0.19f, 0.0f)
    );
    stem.hasMesh = true;
    stem.size = { 0.07f, 0.38f, 0.07f };
    stem.color = { 0.10f, 0.12f, 0.15f };

    SceneNode pivot;
    pivot.localTransform = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(0.0f, 0.19f, 0.0f)
    );

    SceneNode arm;
    arm.localTransform = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(-0.16f, 0.0f, 0.0f)
    );
    arm.hasMesh = true;
    arm.size = { 0.32f, 0.06f, 0.06f };
    arm.color = { 0.10f, 0.12f, 0.15f };

    SceneNode shade;
    shade.localTransform = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(-0.14f, 0.05f, 0.0f)
    );
    shade.hasMesh = true;
    shade.size = { 0.28f, 0.18f, 0.24f };
    shade.color = { 0.95f, 0.72f, 0.28f };

    arm.children.push_back(shade);
    pivot.children.push_back(arm);
    stem.children.push_back(pivot);
    lampRoot.children.push_back(stem);

    buildCat();
}

Room::~Room() {
    delete cube;
}

void Room::add(
    const std::string& name,
    glm::vec3 position,
    glm::vec3 size,
    glm::vec3 color,
    const Texture* texture,
    float tileSize
) {
    const glm::vec3 repeat =
        tileSize > 0.0f ? size / tileSize : glm::vec3(1.0f);

    if (texture && !texture->isLoaded()) {
        texture = nullptr;
    }

    objects.push_back({ name, position, size, color, texture, repeat });
}

void Room::rotateLampArm(float degrees) {
    lampArmAngle = glm::clamp(lampArmAngle + degrees, -60.0f, 60.0f);

    SceneNode& pivot = lampRoot.children[0].children[0];

    pivot.localTransform = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(0.0f, 0.19f, 0.0f)
    );
    pivot.localTransform = glm::rotate(
        pivot.localTransform,
        glm::radians(lampArmAngle),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
}

glm::vec3 Room::getLampLightPosition() const {
    const SceneNode& stem = lampRoot.children[0];
    const SceneNode& pivot = stem.children[0];
    const SceneNode& arm = pivot.children[0];
    const SceneNode& shade = arm.children[0];

    glm::mat4 world = lampRoot.localTransform;
    world = world * stem.localTransform;
    world = world * pivot.localTransform;
    world = world * arm.localTransform;
    world = world * shade.localTransform;

    const glm::vec4 position =
        world * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    return glm::vec3(position);
}

void Room::setLampOn(bool on) {
    // lampRoot -> stem -> pivot -> arm -> shade
    SceneNode& shade = lampRoot.children[0].children[0].children[0].children[0];
    shade.emissive = on ? glm::vec3(0.85f, 0.62f, 0.30f) : glm::vec3(0.0f);
}

void Room::toggleLaptopScreen() {
    laptopScreenOn = !laptopScreenOn;
}

void Room::buildWalls() {
    add("floor", { 0.0f, -0.05f, 0.0f }, { 6.0f, 0.1f, 6.0f }, FLOOR_COLOR, woodFloor.get(), 1.0f);
    add("left wall", { -3.05f, 1.5f, 0.0f }, { 0.1f, 3.0f, 6.0f }, WALL_COLOR, wallPlaster.get(), 1.0f);
    add("back wall left", { -1.75f, 1.5f, -3.05f }, { 2.5f, 3.0f, 0.1f }, WALL_COLOR, wallPlaster.get(), 1.0f);
    add("back wall right", { 2.25f, 1.5f, -3.05f }, { 1.5f, 3.0f, 0.1f }, WALL_COLOR, wallPlaster.get(), 1.0f);
    add("back wall bottom", { 0.5f, 0.5f, -3.05f }, { 2.0f, 1.0f, 0.1f }, WALL_COLOR, wallPlaster.get(), 1.0f);
    add("back wall top", { 0.5f, 2.65f, -3.05f }, { 2.0f, 0.7f, 0.1f }, WALL_COLOR, wallPlaster.get(), 1.0f);
    add("window sill", { 0.5f, 0.97f, -2.93f }, { 2.2f, 0.06f, 0.3f }, FRAME_COLOR);
    add("window frame top", { 0.5f, 2.28f, -3.0f }, { 2.0f, 0.05f, 0.12f }, FRAME_COLOR);
    add("window frame left", { -0.48f, 1.65f, -3.0f }, { 0.05f, 1.3f, 0.12f }, FRAME_COLOR);
    add("window frame right", { 1.48f, 1.65f, -3.0f }, { 0.05f, 1.3f, 0.12f }, FRAME_COLOR);
    add("window frame mid", { 0.5f, 1.65f, -3.0f }, { 0.05f, 1.3f, 0.12f }, FRAME_COLOR);
}

void Room::buildWallArt() {
    const glm::vec3 center(-1.9f, 1.8f, -2.98f);
    add("saturn frame", center, { 0.98f, 0.98f, 0.04f }, { 0.20f, 0.12f, 0.07f }, woodGrain.get(), 0.5f);
    add("saturn canvas", center, { 0.86f, 0.86f, 0.044f }, { 1.0f, 1.0f, 1.0f }, saturnArt.get(), 0.0f);
}

void Room::buildAidanaObjects() {
    add("rug", { -0.2f, 0.01f, 0.5f }, { 3.5f, 0.02f, 4.0f }, { 1.0f, 1.0f, 1.0f }, rugTexture.get(), 0.0f);

    add("bed frame", { -1.9f, 0.2f, -1.0f }, { 1.8f, 0.4f, 2.6f }, { 0.48f, 0.27f, 0.13f }, woodGrain.get(), 0.5f);
    add("bed headboard", { -1.9f, 0.55f, -2.35f }, { 1.8f, 1.1f, 0.1f }, { 0.42f, 0.23f, 0.11f }, woodGrain.get(), 0.5f);
    add("bed mattress", { -1.9f, 0.45f, -1.0f }, { 1.7f, 0.15f, 2.5f }, { 0.9f, 0.9f, 0.95f });
    add("bed blanket", { -1.9f, 0.47f, -0.2f }, { 1.72f, 0.15f, 1.7f }, { 0.2f, 0.4f, 0.6f });
    add("bed pillow", { -1.9f, 0.52f, -2.0f }, { 1.2f, 0.12f, 0.5f }, { 0.8f, 0.85f, 0.9f });

    add("shelf left", { 2.1f, 1.2f, -2.7f }, { 0.05f, 2.4f, 0.6f }, { 0.35f, 0.2f, 0.1f }, woodGrain.get(), 0.5f);
    add("shelf right", { 2.9f, 1.2f, -2.7f }, { 0.05f, 2.4f, 0.6f }, { 0.35f, 0.2f, 0.1f }, woodGrain.get(), 0.5f);
    add("shelf top", { 2.5f, 2.4f, -2.7f }, { 0.85f, 0.05f, 0.6f }, { 0.35f, 0.2f, 0.1f }, woodGrain.get(), 0.5f);
    add("shelf bottom", { 2.5f, 0.05f, -2.7f }, { 0.85f, 0.05f, 0.6f }, { 0.35f, 0.2f, 0.1f }, woodGrain.get(), 0.5f);
    add("shelf mid 1", { 2.5f, 0.8f, -2.7f }, { 0.85f, 0.05f, 0.55f }, { 0.35f, 0.2f, 0.1f }, woodGrain.get(), 0.5f);
    add("shelf mid 2", { 2.5f, 1.6f, -2.7f }, { 0.85f, 0.05f, 0.55f }, { 0.35f, 0.2f, 0.1f }, woodGrain.get(), 0.5f);
    add("book red", { 2.3f, 0.95f, -2.7f }, { 0.1f, 0.25f, 0.2f }, { 0.8f, 0.1f, 0.2f });
    add("book green", { 2.45f, 0.935f, -2.7f }, { 0.08f, 0.22f, 0.2f }, { 0.2f, 0.7f, 0.3f });
    add("book blue", { 2.7f, 1.765f, -2.7f }, { 0.12f, 0.28f, 0.22f }, { 0.1f, 0.2f, 0.8f });

    add("plant pot", { -2.5f, 0.25f, 2.5f }, { 0.4f, 0.5f, 0.4f }, { 0.5f, 0.5f, 0.5f });
    add("plant leaves 1", { -2.5f, 0.65f, 2.5f }, { 0.7f, 0.4f, 0.7f }, { 0.2f, 0.55f, 0.2f });
    add("plant leaves 2", { -2.5f, 0.95f, 2.5f }, { 0.5f, 0.4f, 0.5f }, { 0.25f, 0.65f, 0.25f });
    add("plant leaves 3", { -2.5f, 1.25f, 2.5f }, { 0.3f, 0.3f, 0.3f }, { 0.3f, 0.75f, 0.3f });
}

void Room::draw(const Shader& shader, bool skipLamp) const {
    shader.setInt("uTexture", COLOR_TEXTURE_UNIT);
    const Texture* bound = nullptr;

    for (const RoomObject& obj : objects) {
        const glm::mat4 model =
            glm::scale(
                glm::translate(glm::mat4(1.0f), obj.position),
                obj.size
            );

        shader.setMat4("uModel", model);
        shader.setMat3(
            "uNormalMatrix",
            glm::mat3(glm::transpose(glm::inverse(model)))
        );

        glm::vec3 color = obj.color;
        if (obj.name == "laptop display") {
            color = laptopScreenOn
                ? glm::vec3(0.08f, 0.55f, 1.0f)
                : glm::vec3(0.01f, 0.015f, 0.025f);
        }

        // a screen that is on gives off its own light, like the lamp shade
        const bool glowingScreen = obj.name == "laptop display" && laptopScreenOn;
        shader.setVec3("uColor", color);
        shader.setVec3("uEmissive", glowingScreen ? color * 0.6f : glm::vec3(0.0f));
        shader.setInt("uUseTexture", obj.texture ? 1 : 0);

        if (obj.texture) {
            shader.setVec3("uTexRepeat", obj.texRepeat);

            if (obj.texture != bound) {
                obj.texture->bind(COLOR_TEXTURE_UNIT);
                bound = obj.texture;
            }
        }

        cube->draw();
    }

    drawNode(catRoot, glm::mat4(1.0f), shader, cube);

    if (!skipLamp) {
        drawNode(lampRoot, glm::mat4(1.0f), shader, cube);
    }
}