#ifndef __dxgicommon_h__
#if defined(__cplusplus) && defined(__MINGW32__)
#define __dxgicommon_h__
#pragma push_macro("__CRT_UUID_DECL")
#undef __CRT_UUID_DECL
#include "dxgicommon_gnu.h"
#pragma pop_macro("__CRT_UUID_DECL")
#else
#include "dxgicommon_msvc.h"
#endif

#endif
