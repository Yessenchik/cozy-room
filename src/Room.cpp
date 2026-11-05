#include "Room.h"
#include <glm/gtc/matrix_transform.hpp>

const glm::vec3 FLOOR_COLOR(0.62f, 0.42f, 0.26f);
const glm::vec3 WALL_COLOR(0.56f, 0.70f, 0.90f);
const glm::vec3 FRAME_COLOR(0.95f, 0.95f, 0.92f);

Room::Room() {
    cube = Mesh::createCube();
    buildWalls();

    add("desk top",
        { 1.65f, 0.82f, 1.35f },
        { 1.8f, 0.12f, 1.0f },
        { 0.48f, 0.27f, 0.13f });

    add("desk leg FL",
        { 0.85f, 0.40f, 0.95f },
        { 0.10f, 0.8f, 0.10f },
        { 0.30f, 0.16f, 0.08f });

    add("desk leg FR",
        { 2.45f, 0.40f, 0.95f },
        { 0.10f, 0.8f, 0.10f },
        { 0.30f, 0.16f, 0.08f });

    add("desk leg BL",
        { 0.85f, 0.40f, 1.75f },
        { 0.10f, 0.8f, 0.10f },
        { 0.30f, 0.16f, 0.08f });

    add("desk leg BR",
        { 2.45f, 0.40f, 1.75f },
        { 0.10f, 0.8f, 0.10f },
        { 0.30f, 0.16f, 0.08f });

    add("chair seat",
        { 1.65f, 0.48f, 2.20f },
        { 0.65f, 0.12f, 0.65f },
        { 0.18f, 0.30f, 0.38f });

    add("chair back",
        { 1.65f, 0.95f, 2.48f },
        { 0.65f, 0.85f, 0.10f },
        { 0.18f, 0.30f, 0.38f });

    add("chair leg FL",
        { 1.38f, 0.23f, 1.95f },
        { 0.08f, 0.46f, 0.08f },
        { 0.12f, 0.12f, 0.13f });

    add("chair leg FR",
        { 1.92f, 0.23f, 1.95f },
        { 0.08f, 0.46f, 0.08f },
        { 0.12f, 0.12f, 0.13f });

    add("chair leg BL",
        { 1.38f, 0.23f, 2.43f },
        { 0.08f, 0.46f, 0.08f },
        { 0.12f, 0.12f, 0.13f });

    add("chair leg BR",
        { 1.92f, 0.23f, 2.43f },
        { 0.08f, 0.46f, 0.08f },
        { 0.12f, 0.12f, 0.13f });

    add("lamp base",
        { 2.25f, 0.94f, 1.35f },
        { 0.38f, 0.06f, 0.30f },
        { 0.10f, 0.12f, 0.15f });

    add("lamp stem",
        { 2.25f, 1.13f, 1.35f },
        { 0.07f, 0.38f, 0.07f },
        { 0.10f, 0.12f, 0.15f });

    add("lamp arm",
        { 2.12f, 1.31f, 1.35f },
        { 0.32f, 0.06f, 0.06f },
        { 0.10f, 0.12f, 0.15f });

    add("lamp shade",
        { 1.98f, 1.43f, 1.35f },
        { 0.28f, 0.18f, 0.24f },
        { 0.95f, 0.72f, 0.28f });
}

Room::~Room() {
    delete cube;
}

void Room::add(
    const std::string& name,
    glm::vec3 position,
    glm::vec3 size,
    glm::vec3 color
) {
    objects.push_back({ name, position, size, color });
}

// Room is 6 x 3 x 6, floor at y = 0, centred on x and z.
// Only the left and back walls exist so we can look inside.
void Room::buildWalls() {
    add("floor",
        { 0.0f, -0.05f, 0.0f },
        { 6.0f, 0.1f, 6.0f },
        FLOOR_COLOR);

    add("left wall",
        { -3.05f, 1.5f, 0.0f },
        { 0.1f, 3.0f, 6.0f },
        WALL_COLOR);

    // Back wall is built from four pieces around the window hole.
    // Hole: x from -0.5 to 1.5, y from 1.0 to 2.3.
    add("back wall left",
        { -1.75f, 1.5f, -3.05f },
        { 2.5f, 3.0f, 0.1f },
        WALL_COLOR);

    add("back wall right",
        { 2.25f, 1.5f, -3.05f },
        { 1.5f, 3.0f, 0.1f },
        WALL_COLOR);

    add("back wall bottom",
        { 0.5f, 0.5f, -3.05f },
        { 2.0f, 1.0f, 0.1f },
        WALL_COLOR);

    add("back wall top",
        { 0.5f, 2.65f, -3.05f },
        { 2.0f, 0.7f, 0.1f },
        WALL_COLOR);

    add("window sill",
        { 0.5f, 0.97f, -2.93f },
        { 2.2f, 0.06f, 0.3f },
        FRAME_COLOR);

    add("window frame top",
        { 0.5f, 2.28f, -3.0f },
        { 2.0f, 0.05f, 0.12f },
        FRAME_COLOR);

    add("window frame left",
        { -0.48f, 1.65f, -3.0f },
        { 0.05f, 1.3f, 0.12f },
        FRAME_COLOR);

    add("window frame right",
        { 1.48f, 1.65f, -3.0f },
        { 0.05f, 1.3f, 0.12f },
        FRAME_COLOR);

    add("window frame mid",
        { 0.5f, 1.65f, -3.0f },
        { 0.05f, 1.3f, 0.12f },
        FRAME_COLOR);
}

void Room::draw(const Shader& shader) const {
    for (const RoomObject& obj : objects) {
        glm::mat4 model =
            glm::translate(glm::mat4(1.0f), obj.position);

        model = glm::scale(model, obj.size);

        shader.setMat4("uModel", model);
        shader.setMat3(
            "uNormalMatrix",
            glm::mat3(glm::transpose(glm::inverse(model)))
        );
        shader.setVec3("uColor", obj.color);

        cube->draw();
    }
}