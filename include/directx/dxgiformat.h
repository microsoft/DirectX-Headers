#ifndef __dxgiformat_h__

#if defined(__cplusplus) && defined(__MINGW32__)
#define __dxgiformat_h__
#pragma push_macro("__CRT_UUID_DECL")
#undef __CRT_UUID_DECL
#include "dxgiformat_gnu.h"
#pragma pop_macro("__CRT_UUID_DECL")
#else
#include "dxgiformat_msvc.h"
#endif

#endif
