//
// Created by Xyndra on 19.11.2024.
//

#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include "GLFW/glfw3.h"

struct VulkanWindowAttributes {
    int32_t width;
    int32_t height;
    const char* title;
};

GLFWwindow* createWindow(int32_t width, int32_t height, const char* title);

class VulkanWindowBoilerplate {
public:
    explicit VulkanWindowBoilerplate(VulkanWindowAttributes* window_attributes);
    ~VulkanWindowBoilerplate();
    void mainLoop(bool (*updateHook)());
private:
    VulkanWindowAttributes* attributes;
    GLFWwindow* window;
    VkSurfaceKHR surface;
    VkDevice vkDevice;
    VkQueue graphicsQueue;
    VkSwapchainKHR swapchain;
    uint32_t imageCount;
    uint32_t imageIndex;
    VkPipelineLayout pipelineLayout;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipeline computePipeline;
    VkDescriptorPool descriptorPool;
    VkImage storageImage;
    VkImageView storageImageView;
    VkDeviceMemory storageImageMemory;
    VkFence submitFence;
    std::vector<VkFence> inFlightFences;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;

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
    void initVulkan();
    void createSurface();
    void cleanup();
    void transitionImageLayoutExistingCB(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
    void transitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
    void recordCommandBuffer(uint32_t width, uint32_t height);

    void waitForFlightFence(uint32_t index) const;

    void createStorageImage(uint32_t width, uint32_t height);
    void render(uint32_t width, uint32_t height);
};