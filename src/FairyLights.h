#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "Mesh.h"
#include "Shader.h"

// A string of fairy lights hung over the bed, along the back and the left wall.
// Every bulb is a small point light (up to MAX_BULBS) that basic.frag adds to the
// Blinn-Phong result, and a small glowing cube on a thin wire that hangs in arcs.
// F toggles the string; it also switches on by itself in the evening (see main.cpp).
// When it is switched on, the bulbs light up one after another along the string.
class FairyLights {
public:
    static const int MAX_BULBS = 12;   // must match MAX_FAIRY in basic.frag

    FairyLights();
    ~FairyLights();

    void setOn(bool on);
    void toggle() { setOn(!on); }
    bool isOn() const { return on; }

    // switch-on wave and a gentle twinkle
    void update(float deltaTime);

    // sends bulb positions and colours to basic.frag (call after shader.use())
    void applyLights(const Shader& shader) const;

    // draws the wire and the bulbs with the main shader
    void draw(const Shader& shader) const;

private:
    struct Bulb {
        glm::vec3 position;
        glm::vec3 color;
        float level = 0.0f;    // 0 = dark, 1 = full brightness
        float phase = 0.0f;    // offset of the twinkle
    };

    void addArc(const glm::vec3& from, const glm::vec3& to, float sag, int bulbCount);
    float brightness(const Bulb& bulb) const;

    std::vector<Bulb> bulbs;
    std::vector<glm::mat4> wireSegments;
    Mesh* cube = nullptr;

    bool on = false;
    float sinceSwitch = 0.0f;
    float time = 0.0f;
};
