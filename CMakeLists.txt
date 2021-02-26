# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.10.2)
project(DirectX-Headers
        LANGUAGES CXX
        VERSION 1.0.2)
set(CMAKE_CXX_STANDARD 14) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TEST "Build the test" ON)

include(GNUInstallDirs)

# Enables consumers to add this library as a link target to automatically add
# these include directories, regardless of whether this is referenced via subdirectory
# or from an installed location
add_library(DirectX-Headers INTERFACE)
target_include_directories(DirectX-Headers SYSTEM INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# For non-Windows targets, also add the WSL stubs to the include path
if (NOT WIN32)
    target_include_directories(DirectX-Headers SYSTEM INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/wsl/stubs>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/wsl/stubs>"
    )
endif()

add_library(DirectX-Guids STATIC src/dxguids.cpp)
target_link_libraries(DirectX-Guids PRIVATE DirectX-Headers)

# Install the targets
install(TARGETS DirectX-Headers DirectX-Guids
        EXPORT DirectX-Headers-Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
# Create the targets CMake file which contains the above definitions
install(EXPORT DirectX-Headers-Targets FILE directx-headers-targets.cmake
        NAMESPACE Microsoft::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/directx-headers/cmake)

# Install the actual includes
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Create the CMake config files
include(CMakePackageConfigHelpers)
write_basic_package_version_file("directx-headers-config-version.cmake"
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY SameMajorVersion)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/directx-headers-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/directx-headers-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/directx-headers/cmake)

# Install the CMake config files
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/directx-headers-config.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/directx-headers-config-version.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/directx-headers/cmake)

if (BUILD_TEST)
    add_subdirectory(test)
endif()
