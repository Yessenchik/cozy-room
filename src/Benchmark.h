#pragma once
#include <string>
#include "Camera.h"
#include "DayNight.h"

struct GLFWwindow;

// FPS benchmark for the README table (key B).
// For 20 seconds the camera flies once around the room and the clock runs
// through a whole day, with vsync off. At the end one table row is printed
// to the console and shown on screen.
class Benchmark {
public:
    void start(GLFWwindow* window, DayNight& dayNight);
    void update(GLFWwindow* window, float deltaTime, DayNight& dayNight, Camera& camera);

    bool isRunning() const { return running; }
    // what to show on screen (empty = nothing)
    std::string statusText() const;

private:
    void finish(GLFWwindow* window, DayNight& dayNight);

    static constexpr float DURATION = 20.0f;

    bool running = false;
    float elapsed = 0.0f;
    int frames = 0;
    float slowestFrame = 0.0f;

    // saved state, restored after the run
    float savedHour = 0.0f, savedSpeed = 0.0f;
    bool savedPaused = false;

    std::string result;
    float resultShownFor = 0.0f;
};
