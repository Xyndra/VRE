//
// Created by Xyndra on 19.11.2024.
//

#pragma once

#include <optional>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include "GLFW/glfw3.h"

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    [[nodiscard]] bool isComplete() const {
        return graphicsFamily.has_value();
    }
};

inline VkInstance instance;
inline VkDebugUtilsMessengerEXT debugMessenger;
inline VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
inline VkSurfaceKHR surface;
inline VkDevice vkDevice;
inline VkQueue graphicsQueue;
inline VkSwapchainKHR swapchain;
inline std::vector<VkImage> swapchainImages;
inline std::vector<VkImageView> swapchainImageViews;
inline uint32_t imageCount;
inline uint32_t imageIndex;
inline VkPipelineLayout pipelineLayout;
inline VkCommandPool commandPool;
inline VkCommandBuffer commandBuffer;
inline VkDescriptorSet descriptorSet;
inline VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
inline VkPipeline computePipeline;
inline VkDescriptorPool descriptorPool;
inline VkImage storageImage;
inline VkImageView storageImageView;
inline VkDeviceMemory storageImageMemory;
inline VkFence submitFence;
inline std::vector<VkFence> inFlightFences;

bool isDeviceSuitable(VkPhysicalDevice device);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

void createInstance();
void pickPhysicalDevice();
void createLogicalDevice();
void createSwapchainImageViews();
void presentImage();
void createDescriptorPool();
void allocateDescriptorSet();
void createCommandPool();
void createDescriptorSetLayout();
void createComputePipeline();
void createSwapchain();
void createFences();

void waitNewImage(uint32_t* fenceIndex);
void initVulkan(GLFWwindow* window);
void createSurface(GLFWwindow* window);
void mainLoop(GLFWwindow* window, void (*updateHook)());
void cleanup(GLFWwindow* window);
void transitionImageLayoutExistingCB(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
void transitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
void recordCommandBuffer(uint32_t width, uint32_t height);
void createStorageImage(uint32_t width, uint32_t height);
void render(uint32_t width, uint32_t height);
