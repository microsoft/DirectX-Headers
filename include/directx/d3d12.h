#ifndef __d3d12_h__

#if defined(__cplusplus) && defined(__MINGW32__)
#define __d3d12_h__
#pragma push_macro("__CRT_UUID_DECL")
#undef __CRT_UUID_DECL
#include "d3d12_gnu.h"
#pragma pop_macro("__CRT_UUID_DECL")
#else
#include "d3d12_msvc.h"
#endif

#endif
