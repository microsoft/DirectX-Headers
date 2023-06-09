# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

gtest = cpp.find_library('gtest', required: false)
gtest_main = cpp.find_library('gtest_main', required: false)

if gtest.found() and gtest_main.found()
  feature_support_test = executable('Feature-Support-Test', 'feature_support_test.cpp',
    dependencies : [gtest, gtest_main, dep_dxheaders, d3d12_lib, dxcore_lib],
    cpp_args : test_compile_opts,
    c_args : test_compile_opts)
  test('Feature-Support-Test', feature_support_test)
endif
