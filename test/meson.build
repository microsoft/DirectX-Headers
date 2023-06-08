# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

d3d12_lib = cpp.find_library('d3d12')

headers_test = executable('DirectX-Headers-Test', 'test.cpp',
    dependencies : [dep_dxheaders, d3d12_lib, dxcore_lib],
    cpp_args : test_compile_opts,
    c_args : test_compile_opts)
test('DirectX-Headers-Test', headers_test)

headers_features_test = executable('DirectX-Headers-Check-Feature-Support-Test', 'feature_check_test.cpp',
    dependencies : [dep_dxheaders, d3d12_lib, dxcore_lib],
    cpp_args : test_compile_opts,
    c_args : test_compile_opts)
test('DirectX-Headers-Check-Feature-Support-Test', headers_features_test)

