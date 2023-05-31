# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

project('DirectX-Headers', 'cpp', version : '1.610.0',
        default_options : ['cpp_std=c++14'])

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
