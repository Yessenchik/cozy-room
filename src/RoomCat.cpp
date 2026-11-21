#include "Room.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

// The cat (Aruzhan): a small scene-graph hierarchy whose parts are children of
// catRoot. It changes what it does with the time of day and jumps between places.
// Kept in its own file so that Room.cpp stays under ~500 lines.

namespace {
    SceneNode makeCatPart(
        const glm::vec3& position,
        const glm::vec3& size,
        const glm::vec3& color
    ) {
        SceneNode part;
        part.localTransform = glm::translate(glm::mat4(1.0f), position);
        part.size = size;
        part.color = color;
        part.hasMesh = true;
        return part;
    }

    glm::vec3 catPositionForActivity(int activity) {
        switch (activity) {
        case 0:
            return { 0.92f, 1.02f, -2.88f };
        case 1:
            return { -1.9f, 0.60f, -1.0f };
        case 2:
            return { 0.95f, 0.03f, -0.25f };
        default:
            return { -0.12f, 1.02f, -2.88f };
        }
    }
}

void Room::setTimeOfDay(float hour) {
    timeOfDay = glm::mod(hour, 24.0f);

    if (timeOfDay < 0.0f) {
        timeOfDay += 24.0f;
    }

    int newActivity;

    if (timeOfDay >= 6.0f && timeOfDay < 12.0f) {
        newActivity = 0;
    }
    else if (timeOfDay >= 12.0f && timeOfDay < 18.0f) {
        newActivity = 1;
    }
    else if (timeOfDay >= 18.0f && timeOfDay < 22.0f) {
        newActivity = 2;
    }
    else {
        newActivity = 3;
    }

    if (newActivity == catActivity) {
        return;
    }

    catActivity = newActivity;
    catMoveTarget = catPositionForActivity(catActivity);

    if (catActivity < 0) {
        catPosition = catMoveTarget;
        catMoving = false;
    }
    else if (catMoveElapsed == 0.0f && !catMoving) {
        catPosition = catMoveTarget;
        catMoving = false;
    }
    else {
        catMoveStart = catPosition;
        catMoveElapsed = 0.0f;
        catMoving = true;
    }

    buildCat();
    catRoot.localTransform =
        glm::translate(glm::mat4(1.0f), catPosition);
}

void Room::update(float deltaTime) {
    if (!catMoving) {
        return;
    }

    const float moveDuration = 4.0f;
    catMoveElapsed += glm::max(deltaTime, 0.0f);

    const float linearProgress =
        glm::clamp(catMoveElapsed / moveDuration, 0.0f, 1.0f);

    const float easedProgress =
        linearProgress * linearProgress * (3.0f - 2.0f * linearProgress);

    catPosition = glm::mix(catMoveStart, catMoveTarget, easedProgress);

    const float jumpHeight =
        0.48f * std::sin(linearProgress * 3.14159265f);

    catPosition.y += jumpHeight;
    catRoot.localTransform =
        glm::translate(glm::mat4(1.0f), catPosition);

    if (linearProgress >= 1.0f) {
        catPosition = catMoveTarget;
        catRoot.localTransform =
            glm::translate(glm::mat4(1.0f), catPosition);
        catMoving = false;
        catMoveElapsed = 0.0f;
    }
}

void Room::buildCat() {
    catRoot.children.clear();

    const glm::vec3 fur(0.045f, 0.042f, 0.055f);
    const glm::vec3 furLight(0.075f, 0.068f, 0.085f);
    const glm::vec3 innerEar(0.30f, 0.16f, 0.18f);
    const glm::vec3 muzzle(0.20f, 0.18f, 0.19f);
    const glm::vec3 eye(0.48f, 0.58f, 0.28f);
    const glm::vec3 nose(0.48f, 0.25f, 0.28f);
    const glm::vec3 whiskers(0.40f, 0.38f, 0.38f);

    const bool sleeping = timeOfDay >= 12.0f && timeOfDay < 18.0f;
    const bool playing = timeOfDay >= 18.0f && timeOfDay < 22.0f;

    if (sleeping) {
        catRoot.children.push_back(
            makeCatPart({ 0.0f, 0.02f, 0.0f }, { 0.58f, 0.18f, 0.34f }, fur)
        );
        catRoot.children.push_back(
            makeCatPart({ 0.22f, 0.09f, 0.02f }, { 0.24f, 0.19f, 0.23f }, furLight)
        );
        catRoot.children.push_back(
            makeCatPart({ 0.27f, 0.19f, -0.06f }, { 0.08f, 0.10f, 0.07f }, fur)
        );
        catRoot.children.push_back(
            makeCatPart({ 0.27f, 0.19f, 0.10f }, { 0.08f, 0.10f, 0.07f }, fur)
        );
        catRoot.children.push_back(
            makeCatPart({ -0.24f, 0.06f, 0.02f }, { 0.28f, 0.10f, 0.12f }, furLight)
        );
        return;
    }

    catRoot.children.push_back(
        makeCatPart({ 0.0f, 0.22f, 0.0f }, { 0.27f, 0.36f, 0.24f }, fur)
    );
    catRoot.children.push_back(
        makeCatPart({ 0.0f, 0.30f, 0.02f }, { 0.22f, 0.24f, 0.22f }, furLight)
    );

    catRoot.children.push_back(
        makeCatPart({ -0.11f, 0.04f, -0.07f }, { 0.11f, 0.10f, 0.12f }, furLight)
    );
    catRoot.children.push_back(
        makeCatPart({ 0.11f, 0.04f, -0.07f }, { 0.11f, 0.10f, 0.12f }, furLight)
    );
    catRoot.children.push_back(
        makeCatPart({ -0.11f, 0.04f, 0.07f }, { 0.11f, 0.10f, 0.12f }, furLight)
    );
    catRoot.children.push_back(
        makeCatPart({ 0.11f, 0.04f, 0.07f }, { 0.11f, 0.10f, 0.12f }, furLight)
    );

    catRoot.children.push_back(
        makeCatPart({ 0.0f, 0.53f, 0.0f }, { 0.31f, 0.29f, 0.28f }, fur)
    );

    catRoot.children.push_back(
        makeCatPart({ -0.105f, 0.72f, 0.0f }, { 0.105f, 0.18f, 0.09f }, fur)
    );
    catRoot.children.push_back(
        makeCatPart({ 0.105f, 0.72f, 0.0f }, { 0.105f, 0.18f, 0.09f }, fur)
    );
    catRoot.children.push_back(
        makeCatPart({ -0.105f, 0.72f, 0.047f }, { 0.052f, 0.105f, 0.012f }, innerEar)
    );
    catRoot.children.push_back(
        makeCatPart({ 0.105f, 0.72f, 0.047f }, { 0.052f, 0.105f, 0.012f }, innerEar)
    );

    catRoot.children.push_back(
        makeCatPart({ -0.075f, 0.55f, 0.145f }, { 0.045f, 0.055f, 0.018f }, eye)
    );
    catRoot.children.push_back(
        makeCatPart({ 0.075f, 0.55f, 0.145f }, { 0.045f, 0.055f, 0.018f }, eye)
    );
    catRoot.children.push_back(
        makeCatPart({ 0.0f, 0.48f, 0.15f }, { 0.12f, 0.07f, 0.045f }, muzzle)
    );
    catRoot.children.push_back(
        makeCatPart({ 0.0f, 0.51f, 0.18f }, { 0.035f, 0.025f, 0.018f }, nose)
    );

    catRoot.children.push_back(
        makeCatPart({ -0.18f, 0.49f, 0.16f }, { 0.13f, 0.012f, 0.012f }, whiskers)
    );
    catRoot.children.push_back(
        makeCatPart({ -0.18f, 0.46f, 0.16f }, { 0.13f, 0.012f, 0.012f }, whiskers)
    );
    catRoot.children.push_back(
        makeCatPart({ 0.18f, 0.49f, 0.16f }, { 0.13f, 0.012f, 0.012f }, whiskers)
    );
    catRoot.children.push_back(
        makeCatPart({ 0.18f, 0.46f, 0.16f }, { 0.13f, 0.012f, 0.012f }, whiskers)
    );

    catRoot.children.push_back(
        makeCatPart({ 0.18f, 0.25f, 0.0f }, { 0.20f, 0.07f, 0.08f }, fur)
    );
    catRoot.children.push_back(
        makeCatPart({ 0.27f, 0.31f, 0.0f }, { 0.08f, 0.16f, 0.08f }, furLight)
    );
    catRoot.children.push_back(
        makeCatPart({ 0.29f, 0.39f, 0.0f }, { 0.075f, 0.075f, 0.075f }, fur)
    );

    if (playing) {
        catRoot.children.push_back(
            makeCatPart({ -0.37f, 0.09f, 0.0f }, { 0.14f, 0.12f, 0.14f }, { 0.72f, 0.29f, 0.15f })
        );
        catRoot.children.push_back(
            makeCatPart({ -0.37f, 0.16f, 0.0f }, { 0.025f, 0.025f, 0.025f }, { 0.92f, 0.75f, 0.42f })
        );
    }
}
