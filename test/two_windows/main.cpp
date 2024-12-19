//
// Created by Xyndra on 19.11.2024.
//

#include "../../lib_includes/vre.h"
#include <cstdlib>
#include <iostream>
#include <thread>
#include <chrono>

uint64_t frameCount1 = 0;
std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
bool updateHook1() {
    frameCount1++;
    if (frameCount1 % 60 == 0) {
        std::cout << "Frame: " << frameCount1 << std::endl;
    }

    if (start + std::chrono::seconds(20) < std::chrono::high_resolution_clock::now()) {
        return false;
    }
    return true;
}

uint64_t frameCount2 = 0;
bool updateHook2() {
    frameCount2++;
    if (frameCount2 % 60 == 0) {
        std::cout << "Frame: " << frameCount2 << std::endl;
    }

    if (start + std::chrono::seconds(25) < std::chrono::high_resolution_clock::now()) {
        return false;
    }
    return true;
}

int main() {
    try {
        // Create two threads for the two windows
        std::thread t1([] {
            auto window = VREWindow();
            window.mainLoop(updateHook1);
        });
        std::thread t2([] {
            // glfw will sometimes segfault without a 50ms delay
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            auto window = VREWindow();
            window.mainLoop(updateHook2);
        });
        t1.join();
        t2.join();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}