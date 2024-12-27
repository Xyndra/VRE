//
// Created by Xyndra on 24.11.2024.
//

#include "vulkan_boilerplate.h"
#include "clear_screen_comp.h"
#include <chrono>
#include <iostream>
#include <thread>

void VulkanWindowBoilerplate::createDescriptorPool() {
    VkDescriptorPoolSize poolSize = {
        .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .descriptorCount = 1
    };

    const VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize
    };

    if (vkCreateDescriptorPool(vkDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

void VulkanWindowBoilerplate::allocateDescriptorSet() {
    const VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptorSetLayout
    };

    if (vkAllocateDescriptorSets(vkDevice, &allocInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor set!");
    }

    const VkDescriptorImageInfo imageInfo = {
        .sampler = VK_NULL_HANDLE, // No sampling for storage images
        .imageView = storageImageView,
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL
    };

    const VkWriteDescriptorSet descriptorWrite = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .pImageInfo = &imageInfo
    };

    vkUpdateDescriptorSets(vkDevice, 1, &descriptorWrite, 0, nullptr);
}

void VulkanWindowBoilerplate::createDescriptorSetLayout() {
    constexpr VkDescriptorSetLayoutBinding uboLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .pImmutableSamplers = nullptr
    };

    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &uboLayoutBinding
    };

    if (vkCreateDescriptorSetLayout(vkDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
}

void VulkanWindowBoilerplate::createComputePipeline() {
    const VkShaderModuleCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = clear_screen_spv_len * sizeof(unsigned char),
        .pCode = reinterpret_cast<const uint32_t*>(clear_screen_spv)
    };

    VkShaderModule computeShaderModule;
    if (vkCreateShaderModule(vkDevice, &createInfo, nullptr, &computeShaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module!");
    }

    const VkPipelineShaderStageCreateInfo shaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = computeShaderModule,
        .pName = "main"
    };

    const VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &descriptorSetLayout
    };

    if (vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    const VkComputePipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .stage = shaderStageInfo,
        .layout = pipelineLayout
    };

    if (vkCreateComputePipelines(vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create compute pipeline!");
    }

    vkDestroyShaderModule(vkDevice, computeShaderModule, nullptr);
}

void VulkanWindowBoilerplate::createCommandPool() {
    const VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = 0, // Optional
        .queueFamilyIndex = findQueueFamilies(physicalDevice).graphicsFamily.value()
    };

    if (vkCreateCommandPool(vkDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void VulkanWindowBoilerplate::createStorageImage(const uint32_t width, const uint32_t height) {
    const VkImageCreateInfo imageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .extent = {width, height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    vkCreateImage(vkDevice, &imageInfo, nullptr, &storageImage);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vkDevice, storageImage, &memRequirements);

    const VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };

    vkAllocateMemory(vkDevice, &allocInfo, nullptr, &storageImageMemory);

    vkBindImageMemory(vkDevice, storageImage, storageImageMemory, 0);

    const VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = storageImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM, // Make sure this matches the image format
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    if (vkCreateImageView(vkDevice, &viewInfo, nullptr, &storageImageView) != VK_SUCCESS) {
        throw std::runtime_error("Fehler beim Erstellen des Image Views für das Storage Image");
    }

    transitionImageLayout(vkDevice, commandPool, graphicsQueue, storageImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
}

void VulkanWindowBoilerplate::createFences() {
    constexpr VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = 0
    };

    if (vkCreateFence(vkDevice, &fenceInfo, nullptr, &submitFence) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create fence!");
    }


    inFlightFences.resize(imageCount);
    for (auto & inFlightFence : inFlightFences) {
        if (vkCreateFence(vkDevice, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create fence!");
        }
    }
}

void VulkanWindowBoilerplate::waitNewImage(uint32_t* fenceIndex) {
    *fenceIndex = (*fenceIndex + 1) % imageCount;
    if (firstRender) {
        *fenceIndex = 0;
    }

    if (const VkResult result = vkAcquireNextImageKHR(vkDevice, swapchain, UINT64_MAX, nullptr, inFlightFences[*fenceIndex], &imageIndex); result != VK_SUCCESS) {
        std::cerr << "Failed to acquire next image!" << std::endl;
    }

    if (*fenceIndex != imageIndex) {
        std::cerr << "Fence index does not match image index! (" << *fenceIndex << " != " << imageIndex << ")" << std::endl;
    }
}

void VulkanWindowBoilerplate::recordCommandBuffer(const uint32_t width, const uint32_t height) {
    const VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    if (vkAllocateCommandBuffers(vkDevice, &allocInfo, &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffer!");
    }

    constexpr VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    vkCmdDispatch(commandBuffer, (width + 15) / 16, (height + 15) / 16, 1);

    const VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
        .newLayout = VK_IMAGE_LAYOUT_GENERAL,
        .image = storageImage,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    // copy image to swapchain image
    const VkImageCopy copyRegion = {
        .srcSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .layerCount = 1
        },
        .dstSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .layerCount = 1
        },
        .extent = {
            .width = width,
            .height = height,
            .depth = 1
        }
    };

    transitionImageLayoutExistingCB(commandBuffer, storageImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    transitionImageLayoutExistingCB(commandBuffer, swapchainImages[imageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vkCmdCopyImage(
        commandBuffer,
        storageImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        swapchainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &copyRegion
    );


    const VkImageMemoryBarrier presentBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .image = swapchainImages[imageIndex],
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .layerCount = 1
        }
    };

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &presentBarrier
    );

    transitionImageLayoutExistingCB(commandBuffer, storageImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer!");
    }
}

void VulkanWindowBoilerplate::waitForFlightFence(const uint32_t index) const {
    VkResult result = vkWaitForFences(vkDevice, 1, &inFlightFences[index], VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to wait for fence!" << std::endl;
        result = vkWaitForFences(vkDevice, 1, &inFlightFences[index], VK_TRUE, 1e8);
        if (result != VK_SUCCESS) {
            std::cerr << "Failed to wait for fence twice!" << std::endl;
        }
    }

    result = vkResetFences(vkDevice, 1, &inFlightFences[index]);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to reset fence!" << std::endl;
    }
}

void VulkanWindowBoilerplate::render(const uint32_t width, const uint32_t height) {
    // measure time
    const auto start = std::chrono::high_resolution_clock::now();
    if (firstRender) {
        waitNewImage(&fenceIndex);
        firstRender = false;
    }

    recordCommandBuffer(width, height);

    // const auto recordTime = std::chrono::high_resolution_clock::now();

    const VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer
    };

    waitForFlightFence(fenceIndex);

    // const auto flightFenceTime = std::chrono::high_resolution_clock::now();

    VkResult result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, submitFence);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to submit command buffer!" << std::endl;
    }

    // const auto queueTime = std::chrono::high_resolution_clock::now();

    presentImage();

    // const auto presentTime = std::chrono::high_resolution_clock::now();

    waitNewImage(&fenceIndex);

    // const auto newImageTime = std::chrono::high_resolution_clock::now();

    result = vkWaitForFences(vkDevice, 1, &submitFence, VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to wait for fence!" << std::endl;
    }
    if (result = vkResetFences(vkDevice, 1, &submitFence); result != VK_SUCCESS) {
        std::cerr << "Failed to reset fence!" << std::endl;
    }

    // const auto fenceTime = std::chrono::high_resolution_clock::now();

    vkFreeCommandBuffers(vkDevice, commandPool, 1, &commandBuffer);

    const auto end = std::chrono::high_resolution_clock::now();

    // const std::chrono::duration<double> recordDuration = recordTime - start;
    // const std::chrono::duration<double> flightFenceDuration = flightFenceTime - recordTime;
    // const std::chrono::duration<double> queueDuration = queueTime - flightFenceTime;
    // const std::chrono::duration<double> presentDuration = presentTime - queueTime;
    // const std::chrono::duration<double> newImageDuration = newImageTime - presentTime;
    // const std::chrono::duration<double> fenceDuration = fenceTime - newImageTime;
    const std::chrono::duration<double> totalDuration = end - start;
    // std::cout << "--------------------------------" << std::endl;
    // std::cout << "Record: " << recordDuration.count() << "s" << std::endl;
    // std::cout << "Flight Fence: " << flightFenceDuration.count() << "s" << std::endl;
    // std::cout << "Queue: " << queueDuration.count() << "s" << std::endl;
    // std::cout << "Present: " << presentDuration.count() << "s" << std::endl;
    // std::cout << "New Image: " << newImageDuration.count() << "s" << std::endl;
    // std::cout << "Fence: " << fenceDuration.count() << "s" << std::endl;
    std::cout << "Total: " << totalDuration.count() << "s" << std::endl;
    std::cout << "--------------------------------" << std::endl;
}