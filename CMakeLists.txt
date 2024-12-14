cmake_minimum_required(VERSION 3.29)
project(VRE)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

file(GLOB_RECURSE LIB_SRC_FILES "lib_src/*.cpp")
file(GLOB_RECURSE LIB_INCLUDE_FILES "lib_includes/*.h")

add_library(VRE STATIC ${LIB_SRC_FILES} ${LIB_INCLUDE_FILES})
target_include_directories(VRE PUBLIC lib_includes)
target_link_libraries(VRE
        PRIVATE
            ${Vulkan_LIBRARIES}
            glfw)

add_executable(VRE_EMPTY_TEST
        test/empty/main.cpp)
add_dependencies(VRE_EMPTY_TEST VRE)
target_link_libraries(VRE_EMPTY_TEST
        VRE
        glfw)
