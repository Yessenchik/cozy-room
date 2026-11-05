#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>
#include <iostream>

#include "Camera.h"
#include "Room.h"
#include "Shader.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

Camera camera(glm::vec3(0.0f, 1.2f, 0.0f), 9.0f);
bool rightMouseDown = false;
double lastMouseX = 0.0, lastMouseY = 0.0;

bool lampOn = true;
bool lWasPressed = false;

void onResize(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void onMouseButton(GLFWwindow* window, int button, int action, int) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        rightMouseDown = (action == GLFW_PRESS);
        glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
    }
}

void onMouseMove(GLFWwindow*, double x, double y) {
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

    glEnable(GL_DEPTH_TEST);

    double lastFrameTime = glfwGetTime();
    double fpsTimer = lastFrameTime;
    int frameCount = 0;

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastFrameTime);
        lastFrameTime = currentTime;

        processKeyboard(window, deltaTime);

        glClearColor(0.55f, 0.75f, 0.95f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        float aspect = fbHeight > 0 ? (float)fbWidth / fbHeight : 1.0f;

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = camera.getProjectionMatrix(aspect);

        shader.use();
        shader.setMat4("uView", view);
        shader.setMat4("uProjection", projection);
        shader.setVec3("uViewPos", camera.getPosition());
        shader.setVec3("uLightDir", glm::vec3(0.6f, 1.0f, 0.4f));
        shader.setVec3("uLightColor", glm::vec3(1.0f, 0.95f, 0.85f));

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

        glfwSwapBuffers(window);
        glfwPollEvents();

        frameCount++;

        double fpsNow = glfwGetTime();
        double elapsed = fpsNow - fpsTimer;

        if (elapsed >= 1.0) {
            double fps = frameCount / elapsed;

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