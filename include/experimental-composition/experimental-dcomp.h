/*-------------------------------------------------------------------------------------
 *
 * Copyright (c) Microsoft Corporation
 * Licensed under the MIT license
 *
 * This file defines experimental DirectComposition interfaces. Anything found in this
 * file is under development, and exposed here for testing and experimentation purposes
 * only. The goal is that these APIs will eventually become official APIs after proper
 * testing, experimentation, validation, and stabilization has taken place. However,
 * this is not guaranteed, and everything found here is also subject to change or
 * removal at any time. Do not rely on this functionality from within stable, shipping
 * products.
 *
 *-------------------------------------------------------------------------------------*/

 #if (NTDDI_VERSION >= NTDDI_WIN10_GE)

//+-----------------------------------------------------------------------------
//
//  Interface:
//      EXPERIMENTAL_IDCompositionTextureDirtyRegion
//
//  Synopsis:
//      An interface to manage a dirty region of a composition texture.
//
//------------------------------------------------------------------------------
#undef INTERFACE
#define INTERFACE EXPERIMENTAL_IDCompositionTextureDirtyRegion
DECLARE_INTERFACE_IID_(EXPERIMENTAL_IDCompositionTextureDirtyRegion, IUnknown, "4B14292F-87AC-4A9A-B935-A79B5C74485B")
{
    STDMETHOD(SetDirtyRects)(THIS_
        _In_ size_t rectCount,
        _In_count_(rectCount) const D2D_RECT_L *pRects) PURE;
};

#endif // #if (NTDDI_VERSION >= NTDDI_WIN10_GE)
