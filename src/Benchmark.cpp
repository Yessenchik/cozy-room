#include "Benchmark.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <iostream>

void Benchmark::start(GLFWwindow*, DayNight& dayNight) {
    if (running)
        return;

    savedHour = dayNight.getHour();
    savedSpeed = dayNight.getSpeed();
    savedPaused = dayNight.isPaused();

    // a whole day in DURATION seconds
    dayNight.setHour(6.0f);
    dayNight.setSpeed(24.0f / DURATION);
    dayNight.setPaused(false);

    glfwSwapInterval(0);   // vsync off, otherwise FPS is capped at the monitor rate

    running = true;
    elapsed = 0.0f;
    frames = 0;
    slowestFrame = 0.0f;
    result.clear();
    std::cout << "Benchmark started (" << DURATION << " s)..." << std::endl;
}

void Benchmark::update(GLFWwindow* window, float deltaTime, DayNight& dayNight, Camera& camera) {
    if (!running) {
        if (resultShownFor > 0.0f)
            resultShownFor -= deltaTime;
        return;
    }

    // the first frame after starting can be long (vsync switch), skip it
    if (frames > 0 || elapsed > 0.0f) {
        elapsed += deltaTime;
        if (deltaTime > slowestFrame)
            slowestFrame = deltaTime;
    }
    ++frames;

    camera.rotate(360.0f / DURATION * deltaTime, 0.0f);   // one full circle

    if (elapsed >= DURATION)
        finish(window, dayNight);
}

void Benchmark::finish(GLFWwindow* window, DayNight& dayNight) {
    running = false;
    glfwSwapInterval(1);

    dayNight.setHour(savedHour);
    dayNight.setSpeed(savedSpeed);
    dayNight.setPaused(savedPaused);

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

    float avgFps = (frames - 1) / elapsed;
    float avgMs = 1000.0f * elapsed / (frames - 1);
    float minFps = slowestFrame > 0.0f ? 1.0f / slowestFrame : 0.0f;
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

    char row[256];
    std::snprintf(row, sizeof(row), "| %s | %dx%d | %.0f | %.0f | %.2f ms |",
                  renderer ? renderer : "unknown", fbWidth, fbHeight, avgFps, minFps, avgMs);

    std::cout << "Benchmark finished. Row for the README table:\n"
              << "| GPU | Resolution | Avg FPS | Min FPS | Avg frame time |\n"
              << row << std::endl;

    char shortResult[128];
    std::snprintf(shortResult, sizeof(shortResult),
                  "Benchmark: avg %.0f FPS, min %.0f FPS, %.2f ms/frame (row printed to console)",
                  avgFps, minFps, avgMs);
    result = shortResult;
    resultShownFor = 10.0f;
}

std::string Benchmark::statusText() const {
    if (running) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "Benchmark running... %.0f s left", DURATION - elapsed);
        return buf;
    }
    return resultShownFor > 0.0f ? result : "";
}
