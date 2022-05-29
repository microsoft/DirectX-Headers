#!/bin/bash
export PATH=/c/work/xemu/wmidl/build/src/widl/:$PATH

set -e
set -o xtrace

wmidl -H d3d12.h d3d12.idl -I `cygpath -w /mingw64/include`
wmidl -H d3d12compatibility.h d3d12compatibility.idl -I `cygpath -w /mingw64/include`
wmidl -H d3d12sdklayers.h d3d12sdklayers.idl -I `cygpath -w /mingw64/include`
wmidl -H d3d12video.h d3d12video.idl -I `cygpath -w /mingw64/include`
wmidl -H d3dcommon.h d3dcommon.idl -I `cygpath -w /mingw64/include`
wmidl -H dxgicommon.h dxgicommon.idl -I `cygpath -w /mingw64/include`
wmidl -H dxgiformat.h dxgiformat.idl -I `cygpath -w /mingw64/include`
