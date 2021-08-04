# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
add_subdirectory(googletest)

# Note: Must set the code generation flag to Multi-threaded Debug (/MTd) to compile with Google Test
# Workaround referenced from https://gitlab.kitware.com/cmake/community/-/wikis/FAQ
foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif()
    if(${flag_var} MATCHES "/MDd")
        string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
    endif()
endforeach()

project(DirectX-Headers-GoogleTest-Suite CXX)
add_executable(Feature-Support-Test feature_support_test.cpp)
target_link_libraries(Feature-Support-Test DirectX-Headers DirectX-Guids d3d12 dxcore gtest_main)
