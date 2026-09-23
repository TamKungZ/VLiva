cmake_minimum_required(VERSION 3.16)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(vliva_obs_plugin LANGUAGES CXX)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(VLIVA_OBS_PLUGIN_REQUIRE_OBS "Fail when OBS development files are unavailable" OFF)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(LIBOBS QUIET libobs)
endif()

if(NOT LIBOBS_FOUND)
    if(VLIVA_OBS_PLUGIN_REQUIRE_OBS)
        message(FATAL_ERROR "libobs development files were not found")
    endif()
    message(STATUS "libobs not found: skipping vliva_obs_source")
    return()
endif()

find_path(VLIVA_OBS_MODULE_HEADER_DIR
    NAMES obs-module.h
    HINTS ${LIBOBS_INCLUDE_DIRS}
    PATH_SUFFIXES obs
)
if(NOT VLIVA_OBS_MODULE_HEADER_DIR)
    if(VLIVA_OBS_PLUGIN_REQUIRE_OBS)
        message(FATAL_ERROR "obs-module.h was not found")
    endif()
    message(STATUS "obs-module.h not found: skipping vliva_obs_source")
    return()
endif()

add_library(vliva_obs_source MODULE
    src/shared_frame_consumer.cpp
    src/vliva_obs_source.cpp
)
set_target_properties(vliva_obs_source PROPERTIES
    PREFIX ""
    OUTPUT_NAME "vliva_obs_source"
)
target_include_directories(vliva_obs_source PRIVATE
    include
    ${LIBOBS_INCLUDE_DIRS}
    ${VLIVA_OBS_MODULE_HEADER_DIR}
)
target_link_directories(vliva_obs_source PRIVATE ${LIBOBS_LIBRARY_DIRS})

# Some libobs pkg-config files publish C-only standard flags. Passing those to
# C++ makes otherwise valid plugin builds fail, so retain only compatible flags.
set(LIBOBS_CXX_CFLAGS_OTHER)
foreach(_libobs_flag IN LISTS LIBOBS_CFLAGS_OTHER)
    if(NOT _libobs_flag MATCHES "^-std=")
        list(APPEND LIBOBS_CXX_CFLAGS_OTHER "${_libobs_flag}")
    endif()
endforeach()
target_compile_options(vliva_obs_source PRIVATE ${LIBOBS_CXX_CFLAGS_OTHER})
target_link_libraries(vliva_obs_source PRIVATE ${LIBOBS_LINK_LIBRARIES})

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    include(GNUInstallDirs)
    install(TARGETS vliva_obs_source
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/obs-plugins)
    install(FILES LICENSE README.md
        DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/vliva-obs-plugin)
endif()
