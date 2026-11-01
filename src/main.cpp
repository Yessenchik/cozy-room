#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "Mesh.h"
#include "Shader.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

void onResize(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
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

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClearColor(0.55f, 0.75f, 0.95f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        float aspect = fbHeight > 0 ? (float)fbWidth / fbHeight : 1.0f;

        // TODO: replace with orbit camera
        glm::mat4 view = glm::lookAt(glm::vec3(2.5f, 2.0f, 3.0f), glm::vec3(0.0f), glm::vec3(0, 1, 0));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime() * 0.5f, glm::vec3(0, 1, 0));

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
