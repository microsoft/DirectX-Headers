#!/bin/bash

set -e
set -o xtrace

widl -H d3d12_gnu.h d3d12.idl -I `cygpath -w /mingw64/include`
widl -H d3d12compatibility_gnu.h d3d12compatibility.idl -I `cygpath -w /mingw64/include`
widl -H d3d12sdklayers_gnu.h d3d12sdklayers.idl -I `cygpath -w /mingw64/include`
widl -H d3d12video_gnu.h d3d12video.idl -I `cygpath -w /mingw64/include`
widl -H d3dcommon_gnu.h d3dcommon.idl -I `cygpath -w /mingw64/include`
widl -H dxgicommon_gnu.h dxgicommon.idl -I `cygpath -w /mingw64/include`
widl -H dxgiformat_gnu.h dxgiformat.idl -I `cygpath -w /mingw64/include`
