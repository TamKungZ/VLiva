cmake_minimum_required(VERSION 3.10)
project(VLiva LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Cubism SDK

file(GLOB VLIVA_SRC src/*.cpp)
add_executable(vliva ${VLIVA_SRC})

target_include_directories(vliva PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    /usr/include  
)

target_link_libraries(vliva
    GL
    GLEW
    glfw
    pthread
    dl
)

add_library(vliva_example_plugin MODULE
    plugins/example_plugin.cpp
)
set_target_properties(vliva_example_plugin PROPERTIES
    PREFIX ""
    OUTPUT_NAME "example_plugin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)
target_include_directories(vliva_example_plugin PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)
