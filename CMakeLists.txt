cmake_minimum_required(VERSION 4.2)
project(VkEngine C)

set(CMAKE_C_STANDARD 17)

find_package(Vulkan REQUIRED)

include(FetchContent)
FETCHCONTENT_DECLARE(GLFW GIT_REPOSITORY https://github.com/glfw/glfw.git)
FETCHCONTENT_MAKEAVAILABLE(GLFW)

add_executable(VkEngine src/main.c)

target_link_libraries(VkEngine PRIVATE Vulkan::Vulkan glfw)
