# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
include(FetchContent)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG main # Live at head
)
FetchContent_MakeAvailable(googletest)

list(APPEND dxlibs "")
if(EXISTS "/usr/lib/wsl/lib/")
  find_library(libd3d12 d3d12 HINTS /usr/lib/wsl/lib)
  list(APPEND dxlibs ${libd3d12})
else()
# Fallback to default: let CMake look for libs
  list(APPEND dxlibs d3d12)
endif()

project(DirectX-Headers-GoogleTest-Suite
  DESCRIPTION "DirectX-Header tests using GooleTest"
  HOMEPAGE_URL "https://github.com/microsoft/DirectX-Headers/"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(Feature-Support-Test feature_support_test.cpp)
target_link_libraries(Feature-Support-Test DirectX-Headers DirectX-Guids ${dxlibs} gtest_main)
add_test(Feature-Support-Test Feature-Support-Test)

if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    target_compile_options(Feature-Support-Test PRIVATE -Wno-unused-variable)
endif()

if(WIN32)
    target_compile_definitions(Feature-Support-Test PRIVATE _UNICODE UNICODE _WIN32_WINNT=0x0A00)

    if(WINDOWS_STORE)
      target_compile_definitions(Feature-Support-Test PRIVATE WINAPI_FAMILY=WINAPI_FAMILY_APP)
    endif()
endif()
