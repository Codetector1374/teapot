cmake_minimum_required(VERSION 3.16)
project(teapot)

add_library(teapot
        src/tp_device.cpp src/tp_pipeline.cpp src/tp_swap_chain.cpp src/tp_window.cpp
        src/tp_model.cpp src/tp_renderer.cpp src/simple_render_system.cpp inc/simple_render_system.h)

target_compile_definitions(teapot PRIVATE NOMINMAX)

target_include_directories(teapot PUBLIC inc)
target_compile_definitions(teapot PRIVATE VK_USE_PLATFORM_WIN32_KHR)
target_link_libraries(teapot Vulkan::Vulkan glm glfw)
