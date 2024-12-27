//
// Created by Xyndra on 19.11.2024.
//

#include "vulkan_boilerplate.h"
#include <glfw/glfw3.h>

#include <stdexcept>
#include <vector>
#include <iostream>

VulkanWindowBoilerplate::VulkanWindowBoilerplate(VulkanWindowAttributes *window_attributes) {
    attributes = window_attributes;
    window = createWindow(attributes->width, attributes->height, attributes->title);
    initVulkan();
}

void VulkanWindowBoilerplate::initVulkan() {
    createInstance();
    pickPhysicalDevice();
    setupDebugMessenger();
    createSurface();
    createLogicalDevice();
    createSwapchain();
    createSwapchainImageViews();
    createFences();
    createCommandPool();
    createStorageImage(attributes->width, attributes->height);
    createDescriptorPool();
    createDescriptorSetLayout();
    allocateDescriptorSet();
    createComputePipeline();
}

void VulkanWindowBoilerplate::mainLoop(bool (*updateHook)()) {
    try {
        bool shouldContinue = true;
        while (!glfwWindowShouldClose(window) && shouldContinue) {
            glfwPollEvents();
            shouldContinue = updateHook();
            glfwGetWindowSize(window, &attributes->width, &attributes->height);
            render(attributes->width, attributes->height);
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

VulkanWindowBoilerplate::~VulkanWindowBoilerplate() {
    cleanup();
}

void VulkanWindowBoilerplate::cleanup() {
    vkDeviceWaitIdle(vkDevice);
    for (const auto fence : inFlightFences) {
        vkDestroyFence(vkDevice, fence, nullptr);
    }
    vkDestroyFence(vkDevice, submitFence, nullptr);
    for (const auto imageView : swapchainImageViews) {
        vkDestroyImageView(vkDevice, imageView, nullptr);
    }
    vkDestroyImageView(vkDevice, storageImageView, nullptr);
    vkDestroyImage(vkDevice, storageImage, nullptr);
    vkFreeMemory(vkDevice, storageImageMemory, nullptr);
    vkDestroySwapchainKHR(vkDevice, swapchain, nullptr);
    vkDestroyPipeline(vkDevice, computePipeline, nullptr);
    vkDestroyPipelineLayout(vkDevice, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(vkDevice, descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(vkDevice, descriptorPool, nullptr);
    vkDestroyCommandPool(vkDevice, commandPool, nullptr);
    vkDestroyDevice(vkDevice, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    cleanupDebugMessenger();
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
}

void VulkanWindowBoilerplate::createLogicalDevice() {
    auto [graphicsFamily] = findQueueFamilies(physicalDevice);

    constexpr float queuePriority = 1.0f;
    const VkDeviceQueueCreateInfo queueCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = graphicsFamily.value(),
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    constexpr VkPhysicalDeviceFeatures deviceFeatures = {};
    const std::vector deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    const VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &deviceFeatures
    };

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &vkDevice) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    }

    vkGetDeviceQueue(vkDevice, graphicsFamily.value(), 0, &graphicsQueue);
}