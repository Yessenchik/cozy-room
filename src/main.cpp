#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "Camera.h"
#include "Mesh.h"
#include "Shader.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

Camera camera(glm::vec3(0.0f), 5.0f);
bool rightMouseDown = false;
double lastMouseX = 0.0, lastMouseY = 0.0;

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
        camera.rotate((float)(x - lastMouseX) * sensitivity, (float)(y - lastMouseY) * sensitivity);
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

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Cozy Room", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
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

    Shader shader(std::string(PROJECT_DIR) + "shaders/basic.vert",
                  std::string(PROJECT_DIR) + "shaders/basic.frag");
    Mesh* cube = Mesh::createCube();

    glEnable(GL_DEPTH_TEST);

    float lastFrameTime = (float)glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        processKeyboard(window, deltaTime);

        glClearColor(0.55f, 0.75f, 0.95f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        float aspect = fbHeight > 0 ? (float)fbWidth / fbHeight : 1.0f;

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = camera.getProjectionMatrix(aspect);
        glm::mat4 model(1.0f);

        shader.use();
        shader.setMat4("uModel", model);
        shader.setMat4("uView", view);
        shader.setMat4("uProjection", projection);
        shader.setVec3("uColor", glm::vec3(0.85f, 0.55f, 0.35f));
        cube->draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete cube;
    glfwTerminate();
    return 0;
}
