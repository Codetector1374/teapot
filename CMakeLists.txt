cmake_minimum_required(VERSION 3.16)
project(teapot)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)
add_subdirectory(lib/glm)
add_subdirectory(lib/glfw)

add_subdirectory(engine)

add_subdirectory(demoApp)

