#include "Room.h"
#include <glm/gtc/matrix_transform.hpp>

const glm::vec3 FLOOR_COLOR(1.0f, 1.0f, 1.0f);   // the floor texture has its own colour
const glm::vec3 WALL_COLOR(0.56f, 0.70f, 0.90f);
const glm::vec3 FRAME_COLOR(0.95f, 0.95f, 0.92f);

const int COLOR_TEXTURE_UNIT = 0;   // unit 1 is the shadow map

Room::Room() {
    cube = Mesh::createCube();

    const std::string dir = std::string(PROJECT_DIR) + "textures/";
    woodFloor = std::make_unique<Texture>(dir + "wood_floor.png");
    wallPlaster = std::make_unique<Texture>(dir + "wall_plaster.png");
    woodGrain = std::make_unique<Texture>(dir + "wood_grain.png");
    rugTexture = std::make_unique<Texture>(dir + "rug.png");

    buildWalls();

    buildAidanaObjects();

    add("desk top", { 1.65f, 0.82f, 1.35f }, { 1.8f, 0.12f, 1.0f }, { 0.48f, 0.27f, 0.13f });
    add("desk leg FL", { 0.85f, 0.40f, 0.95f }, { 0.10f, 0.8f, 0.10f }, { 0.30f, 0.16f, 0.08f });
    add("desk leg FR", { 2.45f, 0.40f, 0.95f }, { 0.10f, 0.8f, 0.10f }, { 0.30f, 0.16f, 0.08f });
    add("desk leg BL", { 0.85f, 0.40f, 1.75f }, { 0.10f, 0.8f, 0.10f }, { 0.30f, 0.16f, 0.08f });
    add("desk leg BR", { 2.45f, 0.40f, 1.75f }, { 0.10f, 0.8f, 0.10f }, { 0.30f, 0.16f, 0.08f });
    add("chair seat", { 1.65f, 0.48f, 2.20f }, { 0.65f, 0.12f, 0.65f }, { 0.18f, 0.30f, 0.38f });
    add("chair back", { 1.65f, 0.95f, 2.48f }, { 0.65f, 0.85f, 0.10f }, { 0.18f, 0.30f, 0.38f });
    add("chair leg FL", { 1.38f, 0.23f, 1.95f }, { 0.08f, 0.46f, 0.08f }, { 0.12f, 0.12f, 0.13f });
    add("chair leg FR", { 1.92f, 0.23f, 1.95f }, { 0.08f, 0.46f, 0.08f }, { 0.12f, 0.12f, 0.13f });
    add("chair leg BL", { 1.38f, 0.23f, 2.43f }, { 0.08f, 0.46f, 0.08f }, { 0.12f, 0.12f, 0.13f });
    add("chair leg BR", { 1.92f, 0.23f, 2.43f }, { 0.08f, 0.46f, 0.08f }, { 0.12f, 0.12f, 0.13f });
    add("lamp base", { 2.25f, 0.94f, 1.35f }, { 0.38f, 0.06f, 0.30f }, { 0.10f, 0.12f, 0.15f });
    add("lamp stem", { 2.25f, 1.13f, 1.35f }, { 0.07f, 0.38f, 0.07f }, { 0.10f, 0.12f, 0.15f });
    add("lamp arm", { 2.12f, 1.31f, 1.35f }, { 0.32f, 0.06f, 0.06f }, { 0.10f, 0.12f, 0.15f });
    add("lamp shade", { 1.98f, 1.43f, 1.35f }, { 0.28f, 0.18f, 0.24f }, { 0.95f, 0.72f, 0.28f });
}

Room::~Room() {
    delete cube;
}

void Room::add(const std::string& name, glm::vec3 position, glm::vec3 size, glm::vec3 color,
               const Texture* texture, float tileSize) {
    glm::vec3 repeat = tileSize > 0.0f ? size / tileSize : glm::vec3(1.0f);
    if (texture && !texture->isLoaded())
        texture = nullptr;   // missing file: fall back to the plain colour
    objects.push_back({ name, position, size, color, texture, repeat });
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

void Room::buildAidanaObjects() {
    // the rug image has the same 3.5 : 4 shape as the rug, so it is laid once (tile 0)
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
        if (skipLamp && obj.name.rfind("lamp", 0) == 0)
            continue;
        glm::mat4 model = glm::translate(glm::mat4(1.0f), obj.position);
        model = glm::scale(model, obj.size);

        shader.setMat4("uModel", model);
        shader.setMat3("uNormalMatrix", glm::mat3(glm::transpose(glm::inverse(model))));
        shader.setVec3("uColor", obj.color);
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
}