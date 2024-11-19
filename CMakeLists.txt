cmake_minimum_required(VERSION 3.29)
project(VRE)

set(CMAKE_CXX_STANDARD 20)

add_library(VRE STATIC lib_src/vre.cpp)
target_include_directories(VRE PUBLIC lib_includes)

add_executable(VRE_EMPTY_TEST
        test/empty_test/main.cpp)
add_dependencies(VRE_EMPTY_TEST VRE)
target_link_libraries(VRE_EMPTY_TEST VRE)
