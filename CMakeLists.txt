cmake_minimum_required(VERSION 4.2)
project(VkEngine C)

set(CMAKE_C_STANDARD 17)

find_package(Vulkan REQUIRED)

include(FetchContent)

FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE)

FetchContent_Declare(cglm
    GIT_REPOSITORY https://github.com/recp/cglm.git
    GIT_TAG        v0.9.6
    GIT_SHALLOW    TRUE
    EXCLUDE_FROM_ALL)  # header-only use; don't build cglm's DLL

FetchContent_MakeAvailable(glfw cglm)

add_executable(VkEngine src/main.c src/utils.h src/toolkit.h 
               src/structs.h src/context.h src/window.h src/renderer.h)

target_link_libraries(VkEngine PRIVATE Vulkan::Vulkan glfw cglm_headers)

# Vulkan uses a 0..1 depth range (cglm defaults to OpenGL's -1..1)
target_compile_definitions(VkEngine PRIVATE CGLM_FORCE_DEPTH_ZERO_TO_ONE)
