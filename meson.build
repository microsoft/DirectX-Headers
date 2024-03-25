# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

project('DirectX-Headers', 'cpp', version : '1.613.1',
        default_options : ['cpp_std=c++14'])

cpp = meson.get_compiler('cpp')
compiler_id = cpp.get_id()
#d3d12_lib is not available in linux
d3d12_lib = cpp.find_library('d3d12', required : host_machine.system() == 'windows', disabler : true)
#dxcore is not available in Mingw-w64
dxcore_lib = cpp.find_library('dxcore', required: compiler_id == 'msvc', disabler : true)
test_compile_opts = []
if host_machine.system() == 'windows'
    test_compile_opts = ['-DUNICODE', '-D_WIN32_WINNT=0x0A00']
endif
if (compiler_id == 'gcc') or (compiler_id == 'clang')
    test_compile_opts += ['-Wno-unused-variable']
endif

inc_dirs = [include_directories('include', is_system : true)]
install_inc_subdirs = ['']
compile_options = []

if host_machine.system() != 'windows'
    inc_dirs += include_directories('include/wsl/stubs', is_system : true)
    install_inc_subdirs += ['', 'wsl/stubs', 'directx']
elif meson.get_compiler('cpp').get_id() != 'msvc' and meson.get_compiler('cpp').get_id() != 'clang-cl'
    # MinGW has RPC headers which define old versions, and complain if D3D
    # headers are included before the RPC headers, since D3D headers were
    # generated with new MIDL and "require" new RPC headers.
    compile_options = ['-D__REQUIRED_RPCNDR_H_VERSION__=475']
endif

format_properties_lib = static_library(
    'd3dx12-format-properties',
    'src/d3dx12_property_format_table.cpp',
     include_directories : [inc_dirs, 'include/directx'],
     cpp_args : compile_options,
     install : true)
guids_lib = static_library('DirectX-Guids', 'src/dxguids.cpp', include_directories : inc_dirs, install : true)

dep_dxheaders = declare_dependency(
    link_with : [format_properties_lib, guids_lib],
    include_directories : inc_dirs,
    compile_args : compile_options)

if meson.version().version_compare('>=0.54.0')
    meson.override_dependency('DirectX-Headers', dep_dxheaders)
endif

if not meson.is_subproject() and get_option('build-test')
    subdir('googletest')
    subdir('test')
endif

pkg = import('pkgconfig')
pkg.generate(name : 'DirectX-Headers',
             description : 'Headers for using D3D12',
             url: 'https://github.com/microsoft/DirectX-Headers',
             libraries : [format_properties_lib, guids_lib],
             version : meson.project_version(),
             subdirs : install_inc_subdirs)
install_subdir('include', install_dir : '')
