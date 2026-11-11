#include "DayNight.h"
#include <cmath>

namespace {

const float PI = 3.14159265f;

// Colour key frames over 24 hours. Between two key frames the colours are blended
// smoothly, which gives the warm sunrise/sunset and the cold blue night.
struct KeyFrame {
    float hour;
    glm::vec3 sky;
    glm::vec3 light;
    glm::vec3 ambient;
};

const KeyFrame KEYS[] = {
    // hour   sky                         main light                  ambient
    { 0.0f, {0.02f, 0.03f, 0.08f}, {0.22f, 0.28f, 0.45f}, {0.08f, 0.09f, 0.15f}},   // night (moon)
    { 4.5f, {0.02f, 0.03f, 0.08f}, {0.22f, 0.28f, 0.45f}, {0.08f, 0.09f, 0.15f}},
    { 5.5f, {0.20f, 0.16f, 0.30f}, {0.25f, 0.22f, 0.35f}, {0.13f, 0.12f, 0.17f}},   // dawn
    { 6.5f, {0.98f, 0.62f, 0.40f}, {1.00f, 0.55f, 0.30f}, {0.40f, 0.33f, 0.29f}},   // sunrise
    { 8.0f, {0.60f, 0.78f, 0.96f}, {1.00f, 0.88f, 0.70f}, {0.56f, 0.56f, 0.58f}},   // morning
    {12.0f, {0.52f, 0.74f, 0.98f}, {1.00f, 0.97f, 0.90f}, {0.62f, 0.62f, 0.64f}},   // noon
    {16.0f, {0.58f, 0.74f, 0.92f}, {1.00f, 0.86f, 0.62f}, {0.58f, 0.56f, 0.53f}},   // afternoon
    {17.5f, {0.98f, 0.55f, 0.32f}, {1.00f, 0.50f, 0.22f}, {0.42f, 0.32f, 0.27f}},   // sunset
    {18.5f, {0.30f, 0.18f, 0.35f}, {0.30f, 0.22f, 0.35f}, {0.16f, 0.13f, 0.19f}},   // dusk
    {19.5f, {0.02f, 0.03f, 0.08f}, {0.22f, 0.28f, 0.45f}, {0.08f, 0.09f, 0.15f}},   // night
    {24.0f, {0.02f, 0.03f, 0.08f}, {0.22f, 0.28f, 0.45f}, {0.08f, 0.09f, 0.15f}},
};
const int KEY_COUNT = sizeof(KEYS) / sizeof(KEYS[0]);

float wrapHour(float h) {
    h = std::fmod(h, 24.0f);
    return h < 0.0f ? h + 24.0f : h;
}

float smooth01(float edge0, float edge1, float x) {
    float t = glm::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// The sun goes from 06:00 to 18:00 along an arc behind the back wall (where the window is),
// rising on the left and setting on the right. elevation is in radians, below 0 = under the horizon.
glm::vec3 bodyDirection(float hour, float& elevation) {
    const float maxElevation = glm::radians(50.0f);
    const float maxAzimuth = glm::radians(60.0f);

    float t = (hour - 6.0f) / 12.0f;                 // 0 at rise, 1 at set
    elevation = std::sin(t * PI) * maxElevation;
    float azimuth = (t - 0.5f) * 2.0f * maxAzimuth;  // left -> right

    float e = glm::max(elevation, 0.0f);
    return glm::normalize(glm::vec3(std::cos(e) * std::sin(azimuth),
                                    std::sin(e),
                                    -std::cos(e) * std::cos(azimuth)));
}

}  // namespace

DayNight::DayNight(float startHour) : hour(wrapHour(startHour)) {}

void DayNight::update(float deltaTime) {
    if (!paused)
        hour = wrapHour(hour + speed * deltaTime);
}

void DayNight::setHour(float h) {
    hour = wrapHour(h);
}

const char* DayNight::getPhaseName() const {
    if (hour < 5.0f)  return "Night";
    if (hour < 7.0f)  return "Sunrise";
    if (hour < 11.0f) return "Morning";
    if (hour < 15.0f) return "Noon";
    if (hour < 17.0f) return "Afternoon";
    if (hour < 19.0f) return "Sunset";
    return "Night";
}

bool DayNight::passed(float prevHour, float hour, float mark) {
    if (prevHour <= hour)
        return prevHour < mark && mark <= hour;
    // the clock went over midnight
    return prevHour < mark || mark <= hour;
}

Lighting DayNight::lightingAt(float hour) {
    hour = wrapHour(hour);

    // find the two key frames around this hour and blend them
    int i = 0;
    while (i < KEY_COUNT - 2 && hour >= KEYS[i + 1].hour)
        ++i;
    const KeyFrame& a = KEYS[i];
    const KeyFrame& b = KEYS[i + 1];
    float t = smooth01(a.hour, b.hour, hour);

    Lighting result;
    result.sky = glm::mix(a.sky, b.sky, t);
    result.ambient = glm::mix(a.ambient, b.ambient, t);
    glm::vec3 color = glm::mix(a.light, b.light, t);

    // the sun shines by day, the moon (same arc, 12 hours later) by night
    float sunElevation, moonElevation;
    glm::vec3 sunDir = bodyDirection(hour, sunElevation);
    glm::vec3 moonDir = bodyDirection(wrapHour(hour + 12.0f), moonElevation);

    result.sunUp = sunElevation > 0.0f;
    float elevation = result.sunUp ? sunElevation : moonElevation;
    result.toLight = result.sunUp ? sunDir : moonDir;

    // fade the light out near the horizon so switching sun <-> moon is not visible
    result.lightColor = color * smooth01(0.0f, glm::radians(8.0f), elevation);
    return result;
}
