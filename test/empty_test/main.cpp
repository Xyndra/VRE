//
// Created by Xyndra on 19.11.2024.
//

#include "../../lib_includes/vre.h"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    try {
        initVulkan(window);
        mainLoop(window);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    cleanup(window);

    return EXIT_SUCCESS;
}