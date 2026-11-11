#pragma once
#include <glm/glm.hpp>

// Everything the renderer needs to know about the light at one moment of the day.
struct Lighting {
    glm::vec3 toLight;     // direction towards the main light (sun by day, moon by night)
    glm::vec3 lightColor;  // colour of the main light, already multiplied by its intensity
    glm::vec3 ambient;     // soft fill light from the sky
    glm::vec3 sky;         // background colour (also seen through the window)
    bool sunUp;            // true while the sun is above the horizon
};

// Day/night cycle: keeps the time of day (0..24 hours) and turns it into lighting.
class DayNight {
public:
    explicit DayNight(float startHour = 8.0f);

    // Advances the clock. speed is in game hours per real second.
    void update(float deltaTime);

    float getHour() const { return hour; }
    void setHour(float h);

    float getSpeed() const { return speed; }
    void setSpeed(float s) { speed = s; }

    bool isPaused() const { return paused; }
    void setPaused(bool p) { paused = p; }

    Lighting getLighting() const { return lightingAt(hour); }
    const char* getPhaseName() const;

    // Lighting for any hour, so the HUD can also draw the colours of the whole day.
    static Lighting lightingAt(float hour);

    // True if the clock went past "mark" between prevHour and hour (handles midnight).
    static bool passed(float prevHour, float hour, float mark);

private:
    float hour;
    float speed = 0.2f;   // 0.2 h per second = one full day in 2 minutes
    bool paused = false;
};
