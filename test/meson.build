# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cpp = meson.get_compiler('cpp')
d3d12_lib = cpp.find_library('d3d12')

compiler_id = cpp.get_id()

#dxcore is not available in Mingw-w64
if compiler_id == 'msvc'
  dxcore_lib = cpp.find_library('dxcore')
else
  dxcore_lib = cpp.find_library('dxcore', required: false)
endif

t_compile_opts = []
if host_machine.system() == 'windows'
  t_compile_opts = ['-DUNICODE', '-D_WIN32_WINNT=0x0A00']
endif

if (compiler_id == 'gcc') or (compiler_id == 'clang')
  t_compile_opts += ['-Wno-unused-variable']
endif

headers_test = executable('DirectX-Headers-Test', 'test.cpp',
    dependencies : [dep_dxheaders, d3d12_lib, dxcore_lib],
    cpp_args : t_compile_opts,
    c_args : t_compile_opts)
test('DirectX-Headers-Testz', headers_test)

headers_features_test = executable('DirectX-Headers-Check-Feature-Support-Test', 'feature_check_test.cpp',
    dependencies : [dep_dxheaders, d3d12_lib, dxcore_lib],
    cpp_args : t_compile_opts,
    c_args : t_compile_opts)
test('DirectX-Headers-Check-Feature-Support-Test', headers_features_test)

