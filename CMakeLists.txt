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
