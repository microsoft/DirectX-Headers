# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

list(APPEND dxlibs "")
# Check if in WSL and if has DirectX driver and runtime
if(EXISTS "/usr/lib/wsl/lib/")
  find_library(libd3d12 d3d12 HINTS /usr/lib/wsl/lib)
  find_library(libdxcore dxcore HINTS /usr/lib/wsl/lib)
  list(APPEND dxlibs ${libd3d12} ${libdxcore})
else()
# Fallback to default: let CMake look for libs
  list(APPEND dxlibs d3d12 dxcore)
endif()

project(DirectX-Headers-Test
  DESCRIPTION "DirectX-Header test"
  HOMEPAGE_URL "https://github.com/microsoft/DirectX-Headers/"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(DirectX-Headers-Test test.cpp)
target_link_libraries(DirectX-Headers-Test DirectX-Headers DirectX-Guids ${dxlibs})

add_executable(DirectX-Headers-Check-Feature-Support-Test feature_check_test.cpp)
target_link_libraries(DirectX-Headers-Check-Feature-Support-Test DirectX-Headers DirectX-Guids ${dxlibs})

if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    target_compile_options(DirectX-Headers-Check-Feature-Support-Test PRIVATE -Wno-unused-variable)
endif()
