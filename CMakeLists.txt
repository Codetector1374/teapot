cmake_minimum_required(VERSION 3.16)
project(vk)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

add_subdirectory(lib/glm)
add_subdirectory(lib/glfw)

add_executable(vk src/main.cpp)
target_compile_definitions(vk PRIVATE VK_USE_PLATFORM_WIN32_KHR)
target_link_libraries(vk Vulkan::Vulkan glm glfw)