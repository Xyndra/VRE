//
// Created by Xyndra on 19.11.2024.
//

#include "../../lib_includes/vre.h"
#include <cstdlib>
#include <iostream>
#include <thread>
#include <chrono>

uint64_t frameCount = 0;
std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
bool updateHook() {
    frameCount++;
    if (frameCount % 60 == 0) {
        std::cout << "Frame: " << frameCount << std::endl;
    }

    if (start + std::chrono::seconds(20) < std::chrono::high_resolution_clock::now()) {
        return false;
    }
    return true;
}

int main() {
    auto window = VREWindow();

    try {
        window.mainLoop(updateHook);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}