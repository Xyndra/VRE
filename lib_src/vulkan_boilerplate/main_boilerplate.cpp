//
// Created by Xyndra on 19.11.2024.
//

#include "../../lib_includes/vulkan_boilerplate.h"
#include <glfw/glfw3.h>

#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>
#include <iostream>

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    std::cerr << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

PFN_vkDestroyDebugUtilsMessengerEXT my_vkDestroyDebugUtilsMessengerEXT = nullptr;

void setupDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;

    if (const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
        ); func != nullptr) {
        if (func(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("Failed to set up debug messenger!");
        }
    }

    my_vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
    );
    if (my_vkDestroyDebugUtilsMessengerEXT == nullptr) {
        throw std::runtime_error("Failed to load vkDestroyDebugUtilsMessengerEXT!");
    }
}

void initVulkan(GLFWwindow* window) {
    createInstance();
    setupDebugMessenger();
    createSurface(window);
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
    createSwapchainImageViews();
    createFences();
    createCommandPool();
    createStorageImage(800, 600);
    createDescriptorPool();
    createDescriptorSetLayout();
    allocateDescriptorSet();
    createComputePipeline();
}

void mainLoop(GLFWwindow* window, void (*updateHook)()) {
    int width, height;
    try {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            updateHook();
            glfwGetWindowSize(window, &width, &height);
            render(width, height);
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void cleanup(GLFWwindow* window) {
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
    my_vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("Keine Vulkan-unterstützende GPU gefunden!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Keine geeignete GPU gefunden!");
    }
}

bool isDeviceSuitable(VkPhysicalDevice device) {
    const QueueFamilyIndices indices = findQueueFamilies(device);
    return indices.isComplete();
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

void createLogicalDevice() {
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