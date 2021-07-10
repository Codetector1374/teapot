cmake_minimum_required(VERSION 3.16)
project(vma)

set(CMAKE_CXX_STANDARD 11)

find_package(Vulkan REQUIRED)

add_library(VulkanMemoryAllocator
        src/vk_mem_alloc.cpp)

target_include_directories(VulkanMemoryAllocator PUBLIC inc)
target_link_libraries(VulkanMemoryAllocator PRIVATE Vulkan::Vulkan)