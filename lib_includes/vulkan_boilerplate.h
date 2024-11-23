//
// Created by Xyndra on 19.11.2024.
//

#ifndef VRE_VULKAN_BOILERPLATE_H
#define VRE_VULKAN_BOILERPLATE_H

#include <optional>
#include <cstdint>
#include <vulkan/vulkan.h>
#include "GLFW/glfw3.h"

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    bool isComplete() {
        return graphicsFamily.has_value();
    }
};

void createInstance();
void pickPhysicalDevice();
void createLogicalDevice();

void initVulkan(GLFWwindow* window);
void createSurface(GLFWwindow* window);
void mainLoop(GLFWwindow* window);
void cleanup(GLFWwindow* window);

bool isDeviceSuitable(VkPhysicalDevice device);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

#endif //VRE_VULKAN_BOILERPLATE_H
