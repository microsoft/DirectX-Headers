# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

list(APPEND dxlibs "")
# Check if in WSL and if has DirectX driver and runtime
if(EXISTS "/dev/dxg" AND EXISTS "/lib/wsl/lib/")
  find_library(libd3d12 d3d12 HINTS /lib/wsl/lib)
  find_library(libdxcore dxcore HINTS /lib/wsl/lib)
  list(APPEND dxlibs ${libd3d12} ${libdxcore})
else()
# Fallback to default: let CMake to looking for libs
  list(APPEND dxlibs d3d12 dxcore)
endif()

project(DirectX-Headers-Test CXX)
add_executable(DirectX-Headers-Test test.cpp)
target_link_libraries(DirectX-Headers-Test DirectX-Headers DirectX-Guids ${dxlibs})

add_executable(DirectX-Headers-Check-Feature-Support-Test feature_check_test.cpp)
target_link_libraries(DirectX-Headers-Check-Feature-Support-Test DirectX-Headers DirectX-Guids ${dxlibs})