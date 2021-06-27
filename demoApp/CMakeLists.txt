cmake_minimum_required(VERSION 3.16)
project(demoApp)

include(cmake/GlslShader.cmake)


# Demo App Target
add_executable(teapotDemoApp
        src/main.cpp src/first_app.cpp)

# Shaders
file(GLOB vertex-shaders ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert)
file(GLOB fragment-shaders ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag)
foreach(vertex-shader ${vertex-shaders})
  get_filename_component(p ${vertex-shader} NAME)
  add_shader(teapotDemoApp ${p})
endforeach(vertex-shader)

foreach(fragment-shader ${fragment-shaders})
  get_filename_component(p ${fragment-shader} NAME)
  add_shader(teapotDemoApp ${p})
endforeach(fragment-shader)

target_include_directories(teapotDemoApp PRIVATE inc)
target_link_libraries(teapotDemoApp PRIVATE teapot)
