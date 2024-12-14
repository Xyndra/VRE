//
// Created by Xyndra on 19.11.2024.
//

#include "window_vulkan_boilerplate.h"
#include "global_vulkan_boilerplate.h"
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
    glfwDestroyWindow(window);
}

void VulkanWindowBoilerplate::createLogicalDevice() {
    auto [graphicsFamily] = findQueueFamilies(physicalDevice);

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsFamily.value();
    queueCreateInfo.queueCount = 1;

    constexpr float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    constexpr VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;

    const std::vector deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &vkDevice) != VK_SUCCESS) {
        throw std::runtime_error("Fehler beim Erstellen des logischen Geräts!");
    }

    vkGetDeviceQueue(vkDevice, graphicsFamily.value(), 0, &graphicsQueue);
}