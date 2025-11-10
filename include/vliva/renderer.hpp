#pragma once 

// Forward declaration
struct GLFWwindow; 

namespace vliva {

    /**
     * @brief GLFW
     */
    bool initGLFWWindow(GLFWwindow*& outWin, int width = 1280, int height = 720, const char* title = "VLiva");

    /**
     * @brief Main Loop
     */
    void mainLoop(GLFWwindow* win);

} // namespace vliva
