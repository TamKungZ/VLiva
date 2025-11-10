#include "vliva/renderer.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace vliva {

bool initGLFWWindow(GLFWwindow*& outWin, int width, int height, const char* title) {
    if (!glfwInit()) {
        std::cerr << "glfwInit failed\n";
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    outWin = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!outWin) {
        std::cerr << "glfwCreateWindow failed\n";
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(outWin);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "glewInit failed\n";
        return false;
    }

    glViewport(0, 0, width, height);
    return true;
}

void mainLoop(GLFWwindow* win) {
    while (!glfwWindowShouldClose(win)) {
        glClearColor(0.12f, 0.12f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // (Basic version: No rendering)

        glfwSwapBuffers(win);
        glfwPollEvents();
    }
}

} // namespace vliva
