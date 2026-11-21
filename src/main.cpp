#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>
#include <iostream>
#include <string>

#include "Benchmark.h"
#include "Camera.h"
#include "DayNight.h"
#include "DustParticles.h"
#include "FairyLights.h"
#include "Hud.h"
#include "Mesh.h"
#include "PointShadowMap.h"
#include "Room.h"
#include "Shader.h"
#include "ShadowMap.h"
#include "TimeControls.h"
#include "WindowSky.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

Camera camera(glm::vec3(0.0f, 1.2f, 0.0f), 9.0f);
bool rightMouseDown = false;
double lastMouseX = 0.0;
double lastMouseY = 0.0;

DayNight dayNight(8.0f);
TimeControls timeControls(dayNight);
Benchmark benchmark;
bool vsyncOn = true;
FairyLights* fairyLights = nullptr;

const float LAMP_ON_HOUR = 19.0f;
const float LAMP_OFF_HOUR = 7.0f;
bool lampOn = false;
bool lWasPressed = false;
bool pWasPressed = false;

struct Occluder {
    glm::vec3 position;
    glm::vec3 size;
};

const Occluder SHADOW_ONLY_OCCLUDERS[] = {
    { { 0.0f, 3.05f, 0.0f }, { 6.2f, 0.1f, 6.2f } },
    { { 3.05f, 1.5f, 0.0f }, { 0.1f, 3.0f, 6.0f } }
};

void drawShadowOnlyOccluders(const Shader& depthShader, const Mesh& cube) {
    for (const Occluder& occluder : SHADOW_ONLY_OCCLUDERS) {
        glm::mat4 model =
            glm::translate(glm::mat4(1.0f), occluder.position);
        model = glm::scale(model, occluder.size);
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
    double x;
    double y;
    int width;
    int height;

    glfwGetCursorPos(window, &x, &y);
    glfwGetWindowSize(window, &width, &height);
    timeControls.onMouseButton(button, action, x, y, width, height);

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        rightMouseDown = action == GLFW_PRESS;
        glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
    }
}

void onMouseMove(GLFWwindow* window, double x, double y) {
    int width;
    int height;

    glfwGetWindowSize(window, &width, &height);
    timeControls.onMouseMove(x, y, width, height);

    if (rightMouseDown) {
        const float sensitivity = 0.2f;
        camera.rotate(
            static_cast<float>(x - lastMouseX) * sensitivity,
            static_cast<float>(y - lastMouseY) * sensitivity
        );
    }

    lastMouseX = x;
    lastMouseY = y;
}

void onScroll(GLFWwindow*, double, double yOffset) {
    camera.zoom(static_cast<float>(yOffset) * 0.5f);
}

void onKey(GLFWwindow* window, int key, int, int action, int) {
    timeControls.onKey(key, action);

    if (action != GLFW_PRESS) {
        return;
    }

    if (key == GLFW_KEY_F && fairyLights) {
        fairyLights->toggle();
    }

    if (key == GLFW_KEY_B) {
        benchmark.start(window, dayNight);
    }

    if (key == GLFW_KEY_V && !benchmark.isRunning()) {
        vsyncOn = !vsyncOn;
        glfwSwapInterval(vsyncOn ? 1 : 0);
    }
}

void processKeyboard(GLFWwindow* window, float deltaTime, Room* room) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    const float rotateSpeed = 60.0f * deltaTime;
    const float zoomSpeed = 5.0f * deltaTime;
    const float lampTurnSpeed = 45.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.rotate(rotateSpeed, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.rotate(-rotateSpeed, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        camera.rotate(0.0f, -rotateSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        camera.rotate(0.0f, rotateSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.zoom(zoomSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.zoom(-zoomSpeed);
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS) {
        room->rotateLampArm(lampTurnSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS) {
        room->rotateLampArm(-lampTurnSpeed);
    }

    const bool lIsPressed =
        glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;

    if (lIsPressed && !lWasPressed) {
        lampOn = !lampOn;
    }
    lWasPressed = lIsPressed;

    const bool pIsPressed =
        glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;

    if (pIsPressed && !pWasPressed) {
        room->toggleLaptopScreen();
    }
    pWasPressed = pIsPressed;
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

    if (!gladLoadGLLoader(
        reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to load OpenGL with GLAD\n";
        glfwTerminate();
        return 1;
    }

    int fbWidth;
    int fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    std::cout << "OpenGL " << glGetString(GL_VERSION) << "\n";

    Shader shader(
        std::string(PROJECT_DIR) + "shaders/basic.vert",
        std::string(PROJECT_DIR) + "shaders/basic.frag"
    );

    Room* room = new Room();

    DustParticles* dustParticles = new DustParticles(
        std::string(PROJECT_DIR) + "shaders/dust.vert",
        std::string(PROJECT_DIR) + "shaders/dust.frag",
        1400
    );

    WindowSky windowSky;

    Shader depthShader(
        std::string(PROJECT_DIR) + "shaders/shadow_depth.vert",
        std::string(PROJECT_DIR) + "shaders/shadow_depth.frag"
    );
    ShadowMap shadowMap(2048);
    Mesh* occluderCube = Mesh::createCube();
    const int SHADOW_TEXTURE_UNIT = 1;

    Shader pointShadowShader(
        std::string(PROJECT_DIR) + "shaders/point_shadow.vert",
        std::string(PROJECT_DIR) + "shaders/point_shadow.frag"
    );
    PointShadowMap lampShadowMap(1024);
    const int LAMP_SHADOW_TEXTURE_UNIT = 2;

    fairyLights = new FairyLights();

    Hud hud(
        std::string(PROJECT_DIR) + "shaders/hud.vert",
        std::string(PROJECT_DIR) + "shaders/hud.frag"
    );

    glEnable(GL_DEPTH_TEST);

    lampOn = lampShouldBeOn(dayNight.getHour());
    fairyLights->setOn(lampOn);
    room->setTimeOfDay(dayNight.getHour());

    double lastFrameTime = glfwGetTime();
    double fpsTimer = lastFrameTime;
    int frameCount = 0;
    float fps = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        const double currentTime = glfwGetTime();
        const float deltaTime =
            static_cast<float>(currentTime - lastFrameTime);
        lastFrameTime = currentTime;

        processKeyboard(window, deltaTime, room);
        const glm::vec3 lampPos = room->getLampLightPosition();

        const bool benchmarkWasRunning = benchmark.isRunning();
        benchmark.update(window, deltaTime, dayNight, camera);
        const bool benchmarkJustEnded =
            benchmarkWasRunning && !benchmark.isRunning();

        if (benchmarkJustEnded) {
            glfwSwapInterval(vsyncOn ? 1 : 0);
        }

        const float prevHour = dayNight.getHour();
        timeControls.update(window, deltaTime);
        dayNight.update(deltaTime);
        room->setTimeOfDay(dayNight.getHour());
        room->update(deltaTime);

        if (timeControls.takeJump() || benchmarkJustEnded) {
            lampOn = lampShouldBeOn(dayNight.getHour());
            fairyLights->setOn(lampOn);
        }
        else {
            if (DayNight::passed(
                prevHour, dayNight.getHour(), LAMP_ON_HOUR)) {
                lampOn = true;
                fairyLights->setOn(true);
            }
            if (DayNight::passed(
                prevHour, dayNight.getHour(), LAMP_OFF_HOUR)) {
                lampOn = false;
                fairyLights->setOn(false);
            }
        }

        fairyLights->update(deltaTime);

        const Lighting light = dayNight.getLighting();

        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        shadowMap.begin(light.toLight);
        depthShader.use();
        depthShader.setMat4(
            "uLightSpace",
            shadowMap.getLightSpaceMatrix()
        );
        room->draw(depthShader);
        drawShadowOnlyOccluders(depthShader, *occluderCube);
        shadowMap.end(fbWidth, fbHeight);

        if (lampOn) {
            pointShadowShader.use();
            pointShadowShader.setVec3("uLightPos", lampPos);
            pointShadowShader.setFloat(
                "uFarPlane",
                lampShadowMap.getFarPlane()
            );

            for (int face = 0; face < 6; ++face) {
                pointShadowShader.setMat4(
                    "uLightSpace",
                    lampShadowMap.beginFace(face, lampPos)
                );
                room->draw(pointShadowShader, true);
            }

            lampShadowMap.end(fbWidth, fbHeight);
        }

        glClearColor(light.sky.r, light.sky.g, light.sky.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const float aspect =
            fbHeight > 0
            ? static_cast<float>(fbWidth) / fbHeight
            : 1.0f;

        const glm::mat4 view = camera.getViewMatrix();
        const glm::mat4 projection =
            camera.getProjectionMatrix(aspect);
        const glm::vec3 cameraPosition = camera.getPosition();

        windowSky.drawBackground(
            view,
            projection,
            cameraPosition,
            dayNight.getHour(),
            static_cast<float>(glfwGetTime())
        );

        shader.use();
        shader.setMat4("uView", view);
        shader.setMat4("uProjection", projection);
        shader.setVec3("uViewPos", cameraPosition);
        shader.setVec3("uLightDir", light.toLight);
        shader.setVec3("uLightColor", light.lightColor);
        shader.setVec3("uAmbient", light.ambient);
        shader.setMat4(
            "uLightSpace",
            shadowMap.getLightSpaceMatrix()
        );
        shader.setInt("uShadowMap", SHADOW_TEXTURE_UNIT);
        shadowMap.bindTexture(SHADOW_TEXTURE_UNIT);

        shader.setVec3("uLampPos", lampPos);
        shader.setInt("uLampShadowMap", LAMP_SHADOW_TEXTURE_UNIT);
        shader.setFloat(
            "uLampFarPlane",
            lampShadowMap.getFarPlane()
        );
        lampShadowMap.bindTexture(LAMP_SHADOW_TEXTURE_UNIT);

        shader.setVec3(
            "uLampColor",
            lampOn
            ? glm::vec3(1.0f, 0.72f, 0.38f)
            : glm::vec3(0.0f)
        );

        fairyLights->applyLights(shader);
        room->setLampOn(lampOn);
        room->draw(shader);
        fairyLights->draw(shader);

        const float hour = dayNight.getHour();

        const float morningIntensity =
            glm::clamp((hour - 6.5f) / 1.5f, 0.0f, 1.0f);

        const float eveningIntensity =
            glm::clamp((19.0f - hour) / 1.5f, 0.0f, 1.0f);

        const float sunlightIntensity =
            morningIntensity * eveningIntensity;

        if (sunlightIntensity > 0.01f) {
            const glm::vec3 windowOrigin(0.25f, 2.08f, -2.92f);

            dustParticles->draw(
                windowOrigin,
                view,
                projection,
                static_cast<float>(fbHeight),
                static_cast<float>(glfwGetTime()),
                sunlightIntensity
            );
        }

        windowSky.drawPane(
            view,
            projection,
            cameraPosition,
            dayNight.getHour(),
            static_cast<float>(glfwGetTime())
        );

        int winWidth;
        int winHeight;
        glfwGetWindowSize(window, &winWidth, &winHeight);

        hud.begin(winWidth, winHeight);
        timeControls.draw(
            hud,
            winWidth,
            winHeight,
            fps,
            lampOn,
            fairyLights->isOn()
        );

        const std::string benchText = benchmark.statusText();
        if (!benchText.empty()) {
            const float width =
                Hud::textWidth(benchText, 2.0f) + 24.0f;
            const float x = (winWidth - width) / 2.0f;

            hud.rect(
                x,
                winHeight - 120.0f,
                width,
                36.0f,
                glm::vec4(0.05f, 0.05f, 0.08f, 0.75f)
            );
            hud.textWithShadow(
                x + 12.0f,
                winHeight - 108.0f,
                benchText,
                2.0f,
                glm::vec4(1.0f, 0.8f, 0.45f, 1.0f)
            );
        }

        hud.end();

        glfwSwapBuffers(window);
        glfwPollEvents();

        frameCount++;

        const double fpsNow = glfwGetTime();
        const double elapsed = fpsNow - fpsTimer;

        if (elapsed >= 1.0) {
            fps = static_cast<float>(frameCount / elapsed);

            char title[64];
            std::snprintf(
                title,
                sizeof(title),
                "Cozy Room | %.1f FPS",
                fps
            );
            glfwSetWindowTitle(window, title);

            frameCount = 0;
            fpsTimer = fpsNow;
        }
    }

    delete fairyLights;
    delete dustParticles;
    delete occluderCube;
    delete room;

    glfwTerminate();
    return 0;
}