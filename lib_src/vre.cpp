#include "vre.h"
#include "global_vulkan_boilerplate.h"

#include <iostream>

/**
 * @brief Print "Hello, World!" to the console
 */
void hello() {
    std::cout << "Hello, World!" << std::endl;
}

#define FLIPPING_IF(flag, state, todo) if (flag == state) { todo; flag = !flag; }

bool glfwInitialized = false;
bool globalVulkanInitialized = false;

VREWindow::VREWindow() {
    FLIPPING_IF(glfwInitialized, false, glfwInit());
    FLIPPING_IF(globalVulkanInitialized, false, initGlobalVulkan());
    attributes = VulkanWindowAttributes{800, 600, "Vulkan Raytracing Engine"};
    boilerplate = std::make_shared<VulkanWindowBoilerplate>(&attributes.value());
}

VREWindow::~VREWindow() {
    attributes.reset();
}

void VREWindow::mainLoop(bool (*updateHook)()) {
    boilerplate->mainLoop(updateHook);
}

void cleanUpVRE() {
    FLIPPING_IF(glfwInitialized, true, glfwTerminate());
    FLIPPING_IF(globalVulkanInitialized, true, cleanupGlobalVulkan());
}