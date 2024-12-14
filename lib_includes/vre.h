#pragma once

#include <memory>

#include "window_vulkan_boilerplate.h"
#include <optional>

void hello();

class VREWindow {
public:
    VREWindow();
    ~VREWindow();
    void mainLoop(bool (*updateHook)());
private:
    std::optional<VulkanWindowAttributes> attributes;
    std::shared_ptr<VulkanWindowBoilerplate> boilerplate;
};

void cleanUpVRE();