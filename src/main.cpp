#include <GLFW/glfw3.h>
#include <iostream>
#include "vliva/renderer.hpp"

int main() {
    GLFWwindow* win = nullptr;
    if (!vliva::initGLFWWindow(win)) {
        std::cerr << "Failed to init window\n";
        return 1;
    }

    std::cout << "[VLiva] Basic window created." << std::endl;
    vliva::mainLoop(win);

    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
