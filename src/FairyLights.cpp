#include "FairyLights.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <string>

namespace {

// warm "lo-fi" colours, repeated along the string
const glm::vec3 PALETTE[] = {
    {1.00f, 0.72f, 0.32f},   // amber
    {1.00f, 0.86f, 0.58f},   // warm white
    {1.00f, 0.52f, 0.38f},   // peach
    {0.98f, 0.55f, 0.70f},   // pink
};
const int PALETTE_SIZE = sizeof(PALETTE) / sizeof(PALETTE[0]);

const float LIGHT_INTENSITY = 1.1f;   // how much light one bulb gives to the room
const float BULB_SIZE = 0.06f;
const float WIRE_THICKNESS = 0.012f;
const glm::vec3 WIRE_COLOR(0.06f, 0.07f, 0.05f);
const glm::vec3 GLASS_COLOR(0.30f, 0.26f, 0.20f);   // colour of a bulb that is off

const float WAVE_DELAY = 0.12f;   // seconds between two bulbs when the string switches on
const float FADE_TIME = 0.25f;

// A wire hanging between two hooks sags in the middle; a parabola is close enough
// to the real curve (a catenary) for a short string.
glm::vec3 pointOnArc(const glm::vec3& from, const glm::vec3& to, float sag, float t) {
    glm::vec3 p = glm::mix(from, to, t);
    p.y -= sag * 4.0f * t * (1.0f - t);
    return p;
}

// a thin box from a to b: scale a unit cube to the segment length and turn it
// so that its x axis points from a to b
glm::mat4 segmentMatrix(const glm::vec3& a, const glm::vec3& b) {
    glm::vec3 dir = b - a;
    float length = glm::length(dir);
    glm::vec3 x = dir / length;
    glm::vec3 up = std::fabs(x.y) > 0.99f ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
    glm::vec3 z = glm::normalize(glm::cross(x, up));
    glm::vec3 y = glm::cross(z, x);

    glm::mat4 m(1.0f);
    m[0] = glm::vec4(x, 0.0f);
    m[1] = glm::vec4(y, 0.0f);
    m[2] = glm::vec4(z, 0.0f);
    m[3] = glm::vec4((a + b) * 0.5f, 1.0f);
    return glm::scale(m, glm::vec3(length, WIRE_THICKNESS, WIRE_THICKNESS));
}

}  // namespace

FairyLights::FairyLights() {
    cube = Mesh::createCube();

    // hooks 10 cm in front of the walls (the walls end at x = -3 and z = -3),
    // so the bulbs light the wall around them instead of only grazing it
    const float height = 2.78f;
    const float off = 0.10f;
    const glm::vec3 nearWindow(-0.62f, height, -3.0f + off);
    const glm::vec3 corner(-3.0f + off, height, -3.0f + off);
    const glm::vec3 leftMiddle(-3.0f + off, height, -1.0f);
    const glm::vec3 leftEnd(-3.0f + off, height, 0.9f);

    addArc(nearWindow, corner, 0.22f, 4);   // over the Saturn picture
    addArc(corner, leftMiddle, 0.20f, 4);   // along the left wall, over the bed
    addArc(leftMiddle, leftEnd, 0.20f, 4);
}

FairyLights::~FairyLights() {
    delete cube;
}

void FairyLights::addArc(const glm::vec3& from, const glm::vec3& to, float sag, int bulbCount) {
    // the wire: short straight pieces along the curve
    const int pieces = 12;
    for (int i = 0; i < pieces; ++i) {
        glm::vec3 a = pointOnArc(from, to, sag, float(i) / pieces);
        glm::vec3 b = pointOnArc(from, to, sag, float(i + 1) / pieces);
        wireSegments.push_back(segmentMatrix(a, b));
    }

    // the bulbs, evenly spaced, not on the hooks themselves
    for (int i = 0; i < bulbCount && (int)bulbs.size() < MAX_BULBS; ++i) {
        float t = (i + 0.5f) / bulbCount;
        Bulb bulb;
        bulb.position = pointOnArc(from, to, sag, t) - glm::vec3(0.0f, BULB_SIZE * 0.6f, 0.0f);
        bulb.color = PALETTE[bulbs.size() % PALETTE_SIZE];
        bulb.phase = bulbs.size() * 1.7f;
        bulbs.push_back(bulb);
    }
}

void FairyLights::setOn(bool value) {
    if (value == on)
        return;
    on = value;
    sinceSwitch = 0.0f;
}

void FairyLights::update(float deltaTime) {
    time += deltaTime;
    sinceSwitch += deltaTime;

    for (size_t i = 0; i < bulbs.size(); ++i) {
        Bulb& bulb = bulbs[i];
        if (on) {
            // bulb i starts after i * WAVE_DELAY, so a wave runs along the string
            float t = (sinceSwitch - i * WAVE_DELAY) / FADE_TIME;
            bulb.level = glm::max(bulb.level, glm::clamp(t, 0.0f, 1.0f));
        } else {
            bulb.level = glm::max(0.0f, bulb.level - deltaTime / FADE_TIME);
        }
    }
}

float FairyLights::brightness(const Bulb& bulb) const {
    // a slow, small twinkle so the string looks alive
    float twinkle = 0.88f + 0.12f * std::sin(time * 2.3f + bulb.phase);
    return bulb.level * twinkle;
}

void FairyLights::applyLights(const Shader& shader) const {
    // when every bulb is dark the shader skips the loop completely
    bool anyLit = false;
    for (const Bulb& bulb : bulbs)
        anyLit = anyLit || bulb.level > 0.0f;
    shader.setInt("uFairyCount", anyLit ? (int)bulbs.size() : 0);
    if (!anyLit)
        return;

    for (size_t i = 0; i < bulbs.size(); ++i) {
        std::string index = "[" + std::to_string(i) + "]";
        shader.setVec3("uFairyPos" + index, bulbs[i].position);
        shader.setVec3("uFairyColor" + index, bulbs[i].color * brightness(bulbs[i]) * LIGHT_INTENSITY);
    }
}

void FairyLights::draw(const Shader& shader) const {
    shader.setInt("uUseTexture", 0);
    shader.setVec3("uEmissive", glm::vec3(0.0f));
    shader.setVec3("uColor", WIRE_COLOR);
    for (const glm::mat4& model : wireSegments) {
        shader.setMat4("uModel", model);
        shader.setMat3("uNormalMatrix", glm::mat3(glm::transpose(glm::inverse(model))));
        cube->draw();
    }

    shader.setVec3("uColor", GLASS_COLOR);
    for (const Bulb& bulb : bulbs) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), bulb.position);
        model = glm::scale(model, glm::vec3(BULB_SIZE));
        shader.setMat4("uModel", model);
        shader.setMat3("uNormalMatrix", glm::mat3(glm::transpose(glm::inverse(model))));
        // the bulb glows with its own colour, it does not need light from outside
        shader.setVec3("uEmissive", bulb.color * brightness(bulb) * 1.1f);
        cube->draw();
    }

    shader.setVec3("uEmissive", glm::vec3(0.0f));   // the next object must not glow
}
