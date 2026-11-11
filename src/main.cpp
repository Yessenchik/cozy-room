#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>
#include <iostream>

#include "Camera.h"
#include "DayNight.h"
#include "Hud.h"
#include "Room.h"
#include "Shader.h"
#include "TimeControls.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

Camera camera(glm::vec3(0.0f, 1.2f, 0.0f), 9.0f);
bool rightMouseDown = false;
double lastMouseX = 0.0, lastMouseY = 0.0;

DayNight dayNight(8.0f);   // the scene starts at 08:00
TimeControls timeControls(dayNight);

// the lamp switches on by itself in the evening and off in the morning,
// L still toggles it by hand at any time
const float LAMP_ON_HOUR = 19.0f;
const float LAMP_OFF_HOUR = 7.0f;
bool lampOn = false;
bool lWasPressed = false;

bool lampShouldBeOn(float hour) {
    return hour >= LAMP_ON_HOUR || hour < LAMP_OFF_HOUR;
}

void onResize(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void onMouseButton(GLFWwindow* window, int button, int action, int) {
    double x, y;
    int width, height;
    glfwGetCursorPos(window, &x, &y);
    glfwGetWindowSize(window, &width, &height);
    timeControls.onMouseButton(button, action, x, y, width, height);

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        rightMouseDown = (action == GLFW_PRESS);
        glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
    }
}

void onMouseMove(GLFWwindow* window, double x, double y) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    timeControls.onMouseMove(x, y, width, height);

    if (rightMouseDown) {
        float sensitivity = 0.2f;
        camera.rotate((float)(x - lastMouseX) * sensitivity,
            (float)(y - lastMouseY) * sensitivity);
    }
    lastMouseX = x;
    lastMouseY = y;
}

void onScroll(GLFWwindow*, double, double yOffset) {
    camera.zoom((float)yOffset * 0.5f);
}

void onKey(GLFWwindow*, int key, int, int action, int) {
    timeControls.onKey(key, action);
}

void processKeyboard(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // speeds are per second, so movement is the same at any FPS
    float rotateSpeed = 60.0f * deltaTime;
    float zoomSpeed = 5.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.rotate(rotateSpeed, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.rotate(-rotateSpeed, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) camera.rotate(0.0f, -rotateSpeed);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camera.rotate(0.0f, rotateSpeed);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.zoom(zoomSpeed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.zoom(-zoomSpeed);

    bool lIsPressed = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;

    if (lIsPressed && !lWasPressed) {
        lampOn = !lampOn;
    }

    lWasPressed = lIsPressed;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialise GLFW\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        "Cozy Room",
        nullptr,
        nullptr
    );

    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, onResize);
    glfwSetMouseButtonCallback(window, onMouseButton);
    glfwSetCursorPosCallback(window, onMouseMove);
    glfwSetScrollCallback(window, onScroll);
    glfwSetKeyCallback(window, onKey);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to load OpenGL with GLAD\n";
        return 1;
    }

    // Retina: framebuffer is bigger than the window size
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    std::cout << "OpenGL " << glGetString(GL_VERSION) << "\n";

    Shader shader(
        std::string(PROJECT_DIR) + "shaders/basic.vert",
        std::string(PROJECT_DIR) + "shaders/basic.frag"
    );

    Room* room = new Room();

    Hud hud(
        std::string(PROJECT_DIR) + "shaders/hud.vert",
        std::string(PROJECT_DIR) + "shaders/hud.frag"
    );

    glEnable(GL_DEPTH_TEST);

    lampOn = lampShouldBeOn(dayNight.getHour());

    double lastFrameTime = glfwGetTime();
    double fpsTimer = lastFrameTime;
    int frameCount = 0;
    float fps = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastFrameTime);
        lastFrameTime = currentTime;

        processKeyboard(window, deltaTime);

        float prevHour = dayNight.getHour();
        timeControls.update(window, deltaTime);
        dayNight.update(deltaTime);
        if (timeControls.takeJump()) {
            lampOn = lampShouldBeOn(dayNight.getHour());
        } else {
            if (DayNight::passed(prevHour, dayNight.getHour(), LAMP_ON_HOUR)) lampOn = true;
            if (DayNight::passed(prevHour, dayNight.getHour(), LAMP_OFF_HOUR)) lampOn = false;
        }

        Lighting light = dayNight.getLighting();

        glClearColor(light.sky.r, light.sky.g, light.sky.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        float aspect = fbHeight > 0 ? (float)fbWidth / fbHeight : 1.0f;

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = camera.getProjectionMatrix(aspect);

        shader.use();
        shader.setMat4("uView", view);
        shader.setMat4("uProjection", projection);
        shader.setVec3("uViewPos", camera.getPosition());
        shader.setVec3("uLightDir", light.toLight);
        shader.setVec3("uLightColor", light.lightColor);
        shader.setVec3("uAmbient", light.ambient);

        shader.setVec3(
            "uLampPos",
            glm::vec3(1.98f, 1.43f, 1.35f)
        );

        shader.setVec3(
            "uLampColor",
            lampOn ? glm::vec3(1.0f, 0.72f, 0.38f)
            : glm::vec3(0.0f)
        );

        room->draw(shader);

        int winWidth, winHeight;
        glfwGetWindowSize(window, &winWidth, &winHeight);
        hud.begin(winWidth, winHeight);
        timeControls.draw(hud, winWidth, winHeight, fps, lampOn);
        hud.end();

        glfwSwapBuffers(window);
        glfwPollEvents();

        frameCount++;

        double fpsNow = glfwGetTime();
        double elapsed = fpsNow - fpsTimer;

        if (elapsed >= 1.0) {
            fps = (float)(frameCount / elapsed);

            char title[64];
            std::snprintf(title, sizeof(title), "Cozy Room | %.1f FPS", fps);
            glfwSetWindowTitle(window, title);

            frameCount = 0;
            fpsTimer = fpsNow;
        }
    }

    delete room;
    glfwTerminate();
    return 0;
}
