#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>
#include <iostream>

#include "Benchmark.h"
#include "Camera.h"
#include "DayNight.h"
#include "Hud.h"
#include "Mesh.h"
#include "Room.h"
#include "Shader.h"
#include "ShadowMap.h"
#include "TimeControls.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

Camera camera(glm::vec3(0.0f, 1.2f, 0.0f), 9.0f);
bool rightMouseDown = false;
double lastMouseX = 0.0, lastMouseY = 0.0;

DayNight dayNight(8.0f);   // the scene starts at 08:00
TimeControls timeControls(dayNight);
Benchmark benchmark;
bool vsyncOn = true;

// the lamp switches on by itself in the evening and off in the morning,
// L still toggles it by hand at any time
const float LAMP_ON_HOUR = 19.0f;
const float LAMP_OFF_HOUR = 7.0f;
bool lampOn = false;
bool lWasPressed = false;

// The room is a cut-away: no ceiling and no right wall, so the camera can look in.
// For the sun these parts still exist: they are drawn only into the shadow map,
// which makes the sunlight come into the room only through the window.
struct Occluder { glm::vec3 position, size; };
const Occluder SHADOW_ONLY_OCCLUDERS[] = {
    {{0.0f, 3.05f, 0.0f}, {6.2f, 0.1f, 6.2f}},   // ceiling
    {{3.05f, 1.5f, 0.0f}, {0.1f, 3.0f, 6.0f}},   // right wall
};

void drawShadowOnlyOccluders(const Shader& depthShader, const Mesh& cube) {
    for (const Occluder& o : SHADOW_ONLY_OCCLUDERS) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), o.position);
        model = glm::scale(model, o.size);
        depthShader.setMat4("uModel", model);
        cube.draw();
    }
}

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

void onKey(GLFWwindow* window, int key, int, int action, int) {
    timeControls.onKey(key, action);

    if (action != GLFW_PRESS)
        return;
    if (key == GLFW_KEY_B)
        benchmark.start(window, dayNight);
    if (key == GLFW_KEY_V && !benchmark.isRunning()) {
        vsyncOn = !vsyncOn;
        glfwSwapInterval(vsyncOn ? 1 : 0);
    }
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

    // shadow mapping for the sun / moon
    Shader depthShader(
        std::string(PROJECT_DIR) + "shaders/shadow_depth.vert",
        std::string(PROJECT_DIR) + "shaders/shadow_depth.frag"
    );
    ShadowMap shadowMap(2048);
    Mesh* occluderCube = Mesh::createCube();
    const int SHADOW_TEXTURE_UNIT = 1;   // unit 0 is left free for colour textures

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

        bool benchmarkWasRunning = benchmark.isRunning();
        benchmark.update(window, deltaTime, dayNight, camera);
        bool benchmarkJustEnded = benchmarkWasRunning && !benchmark.isRunning();
        if (benchmarkJustEnded)
            glfwSwapInterval(vsyncOn ? 1 : 0);

        float prevHour = dayNight.getHour();
        timeControls.update(window, deltaTime);
        dayNight.update(deltaTime);
        if (timeControls.takeJump() || benchmarkJustEnded) {
            lampOn = lampShouldBeOn(dayNight.getHour());
        } else {
            if (DayNight::passed(prevHour, dayNight.getHour(), LAMP_ON_HOUR)) lampOn = true;
            if (DayNight::passed(prevHour, dayNight.getHour(), LAMP_OFF_HOUR)) lampOn = false;
        }

        Lighting light = dayNight.getLighting();

        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        // pass 1: depth of the scene as seen from the sun
        shadowMap.begin(light.toLight);
        depthShader.use();
        depthShader.setMat4("uLightSpace", shadowMap.getLightSpaceMatrix());
        room->draw(depthShader);
        drawShadowOnlyOccluders(depthShader, *occluderCube);
        shadowMap.end(fbWidth, fbHeight);

        // pass 2: the normal picture
        glClearColor(light.sky.r, light.sky.g, light.sky.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
        shader.setMat4("uLightSpace", shadowMap.getLightSpaceMatrix());
        shader.setInt("uShadowMap", SHADOW_TEXTURE_UNIT);
        shadowMap.bindTexture(SHADOW_TEXTURE_UNIT);

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

        std::string benchText = benchmark.statusText();
        if (!benchText.empty()) {
            float w = Hud::textWidth(benchText, 2.0f) + 24.0f;
            float x = (winWidth - w) / 2.0f;
            hud.rect(x, winHeight - 120.0f, w, 36.0f, glm::vec4(0.05f, 0.05f, 0.08f, 0.75f));
            hud.textWithShadow(x + 12.0f, winHeight - 108.0f, benchText, 2.0f, glm::vec4(1.0f, 0.8f, 0.45f, 1.0f));
        }
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

    delete occluderCube;
    delete room;
    glfwTerminate();
    return 0;
}
