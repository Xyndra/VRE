//
// Created by Xyndra on 24.11.2024.
//

#include "vulkan_boilerplate.h"
#include "clear_screen_comp.h"
#include <chrono>
#include <iostream>
#include <thread>

void VulkanWindowBoilerplate::createDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(vkDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

void VulkanWindowBoilerplate::allocateDescriptorSet() {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    if (vkAllocateDescriptorSets(vkDevice, &allocInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor set!");
    }

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageInfo.imageView = storageImageView;
    imageInfo.sampler = VK_NULL_HANDLE; // Kein Sampler für Storage Images nötig

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(vkDevice, 1, &descriptorWrite, 0, nullptr);
}

void VulkanWindowBoilerplate::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(vkDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
}

void VulkanWindowBoilerplate::createComputePipeline() {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = clear_screen_spv_len * sizeof(unsigned char);
    createInfo.pCode = reinterpret_cast<const uint32_t*>(clear_screen_spv);

    VkShaderModule computeShaderModule;
    if (vkCreateShaderModule(vkDevice, &createInfo, nullptr, &computeShaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module!");
    }

    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageInfo.module = computeShaderModule;
    shaderStageInfo.pName = "main";

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    if (vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = shaderStageInfo;
    pipelineInfo.layout = pipelineLayout;

    if (vkCreateComputePipelines(vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create compute pipeline!");
    }

    vkDestroyShaderModule(vkDevice, computeShaderModule, nullptr);
}

void VulkanWindowBoilerplate::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = findQueueFamilies(physicalDevice).graphicsFamily.value();
    poolInfo.flags = 0; // Optional

    if (vkCreateCommandPool(vkDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void VulkanWindowBoilerplate::createStorageImage(const uint32_t width, const uint32_t height) {
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.extent = {width, height, 1};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vkCreateImage(vkDevice, &imageInfo, nullptr, &storageImage);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vkDevice, storageImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(vkDevice, &allocInfo, nullptr, &storageImageMemory);

    vkBindImageMemory(vkDevice, storageImage, storageImageMemory, 0);

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = storageImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM; // Stellen Sie sicher, dass dies mit dem Image-Format übereinstimmt
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(vkDevice, &viewInfo, nullptr, &storageImageView) != VK_SUCCESS) {
        throw std::runtime_error("Fehler beim Erstellen des Image Views für das Storage Image");
    }

    transitionImageLayout(vkDevice, commandPool, graphicsQueue, storageImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
}

void VulkanWindowBoilerplate::createFences() {
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = 0;

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

    std::cout << "Starting to wait for fence " << *fenceIndex << "(thread id: " << std::this_thread::get_id() << ")" << std::endl;
}

void VulkanWindowBoilerplate::recordCommandBuffer(const uint32_t width, const uint32_t height) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(vkDevice, &allocInfo, &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffer!");
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    vkCmdDispatch(commandBuffer, (width + 15) / 16, (height + 15) / 16, 1);

    VkImageMemoryBarrier barrier = {};

    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.image = storageImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    // copy image to swapchain image
    VkImageCopy copyRegion{};
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.layerCount = 1;
    copyRegion.extent.width = width;
    copyRegion.extent.height = height;
    copyRegion.extent.depth = 1;

    transitionImageLayoutExistingCB(commandBuffer, storageImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    transitionImageLayoutExistingCB(commandBuffer, swapchainImages[imageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vkCmdCopyImage(
        commandBuffer,
        storageImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        swapchainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &copyRegion
    );


    VkImageMemoryBarrier presentBarrier = {};
    presentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    presentBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    presentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    presentBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    presentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    presentBarrier.image = swapchainImages[imageIndex];
    presentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    presentBarrier.subresourceRange.levelCount = 1;
    presentBarrier.subresourceRange.layerCount = 1;

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
    std::cout << "Idling for fence " << index << "(thread id: " << std::this_thread::get_id() << ")" << std::endl;
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

    const auto recordTime = std::chrono::high_resolution_clock::now();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    waitForFlightFence(fenceIndex);

    const auto flightFenceTime = std::chrono::high_resolution_clock::now();

    VkResult result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, submitFence);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to submit command buffer!" << std::endl;
    }

    const auto queueTime = std::chrono::high_resolution_clock::now();

    presentImage();

    const auto presentTime = std::chrono::high_resolution_clock::now();

    waitNewImage(&fenceIndex);

    const auto newImageTime = std::chrono::high_resolution_clock::now();

    result = vkWaitForFences(vkDevice, 1, &submitFence, VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to wait for fence!" << std::endl;
    }
    if (result = vkResetFences(vkDevice, 1, &submitFence); result != VK_SUCCESS) {
        std::cerr << "Failed to reset fence!" << std::endl;
    }

    const auto fenceTime = std::chrono::high_resolution_clock::now();

    vkFreeCommandBuffers(vkDevice, commandPool, 1, &commandBuffer);

    const auto end = std::chrono::high_resolution_clock::now();

    const std::chrono::duration<double> recordDuration = recordTime - start;
    const std::chrono::duration<double> flightFenceDuration = flightFenceTime - recordTime;
    const std::chrono::duration<double> queueDuration = queueTime - flightFenceTime;
    const std::chrono::duration<double> presentDuration = presentTime - queueTime;
    const std::chrono::duration<double> newImageDuration = newImageTime - presentTime;
    const std::chrono::duration<double> fenceDuration = fenceTime - newImageTime;
    const std::chrono::duration<double> totalDuration = end - start;
    std::cout << "--------------------------------" << std::endl;
    std::cout << "Record: " << recordDuration.count() << "s" << std::endl;
    std::cout << "Flight Fence: " << flightFenceDuration.count() << "s" << std::endl;
    std::cout << "Queue: " << queueDuration.count() << "s" << std::endl;
    std::cout << "Present: " << presentDuration.count() << "s" << std::endl;
    std::cout << "New Image: " << newImageDuration.count() << "s" << std::endl;
    std::cout << "Fence: " << fenceDuration.count() << "s" << std::endl;
    std::cout << "Total: " << totalDuration.count() << "s" << std::endl;
    std::cout << "--------------------------------" << std::endl;
}