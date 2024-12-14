//
// Created by Xyndra on 01.12.2024.
//

#include <iostream>

#include "window_vulkan_boilerplate.h"
#include <stdexcept>

#include "global_vulkan_boilerplate.h"

GLFWwindow* createWindow(const int32_t width, const int32_t height, const char* title) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    return glfwCreateWindow(width, height, title, nullptr, nullptr);
}

void VulkanWindowBoilerplate::createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}