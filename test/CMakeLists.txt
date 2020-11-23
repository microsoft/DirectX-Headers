# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

project(DirectX-Headers-Test CXX)
add_executable(DirectX-Headers-Test test.cpp)
target_link_libraries(DirectX-Headers-Test DirectX-Headers DirectX-Guids d3d12 dxcore)