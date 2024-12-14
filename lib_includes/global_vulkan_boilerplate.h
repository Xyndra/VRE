//
// Created by Xyndra on 14.12.2024.
//

#pragma once
#include <optional>
#include <vulkan/vulkan_core.h>

inline VkInstance instance;

void initGlobalVulkan();
void cleanupGlobalVulkan();
void setupDebugMessenger();
void cleanupDebugMessenger();

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    [[nodiscard]] bool isComplete() const {
        return graphicsFamily.has_value();
    }
};

inline VkPhysicalDevice physicalDevice;

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
bool isDeviceSuitable(VkPhysicalDevice device);
void pickPhysicalDevice();

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);