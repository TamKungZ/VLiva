cmake_minimum_required(VERSION 3.16)
project(VLiva LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)

add_executable(vliva src/main.cpp)
target_link_libraries(vliva PRIVATE Qt6::Widgets)

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

option(VLIVA_BUILD_OBS_PLUGIN "Build the standalone GPL OBS source plugin" ON)
if(VLIVA_BUILD_OBS_PLUGIN)
    add_subdirectory(plugins/obs)
endif()

include(GNUInstallDirs)
install(TARGETS vliva RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES README.md LICENSE
    DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/vliva-public-notice)
