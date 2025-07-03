/*-------------------------------------------------------------------------------------
 *
 * Copyright (c) Microsoft Corporation
 * Licensed under the MIT license
 *
 * dcomp-preview.h - Preview versions of upcoming DirectComposition interfaces.
 *
 * This file provides preview versions of DirectComposition interfaces that may eventually
 * be integrated into the official dcomp.h header of the Windows SDK. The inclusion of an
 * interface in this file does not guarantee its future adoption. However, if an interface is
 * adopted exactly as defined here, the official definition—including its GUID—will be identical
 * to the preview version.
 *
 * If any modifications are made to an interface prior to its official release, the GUID will be
 * updated accordingly. This guarantees that a QueryInterface (QI) call for a PREVIEW_ interface
 * is safe: a QI will only succeed if the GUID matches, and will gracefully fail if the official
 * version differs. 
 *
 * Developers can transition to the official interfaces by removing the "PREVIEW_" prefix, provided
 * that the GUIDs remain the same. Note that some PREVIEW_ interfaces may eventually be deprecated
 * and removed from this file. For a smooth migration, it is recommended to adopt the official
 * interfaces as soon as they become available in the Windows SDK. The preview interfaces will
 * remain available long enough to support this transition.
 * 
 * Always query for these interfaces using QueryInterface and validate the results, rather than
 * assuming that the target Windows system supports the required features.
 *
 *-------------------------------------------------------------------------------------*/
#pragma once

#if (NTDDI_VERSION >= NTDDI_WIN11_GE)

//+-----------------------------------------------------------------------------
//
//  Interface:
//      PREVIEW_IDCompositionDynamicTexture
//
//  Synopsis:
//      An interface representing a dynamically changing texture that can be
//      bound to a dcomp visual as content.
//
//------------------------------------------------------------------------------
#undef INTERFACE
#define INTERFACE PREVIEW_IDCompositionDynamicTexture
DECLARE_INTERFACE_IID_(PREVIEW_IDCompositionDynamicTexture, IUnknown, "A1DE1D3F-6405-447F-8E95-1383A34B0277")
{
    // Set the current texture, assuming that every pixel has changed.
    STDMETHOD(SetTexture)(THIS_
        _In_ IDCompositionTexture* pTexture) PURE;

    // Set the current texture, assuming that only the pixels within the specified dirty rects
    // have changed.
    //
    // DWM will use these dirty rects to optimize redrawing the texture on the screen.
    // However, it does not verify the accuracy of the provided rects, so the caller must ensure
    // that every changed pixel is covered by at least one rect. There are no guarantees regarding
    // the redrawing of pixels outside the specified dirty rects; DWM may choose to redraw
    // additional areas if necessary.
    //
    // If the provided array is empty or if the dirty rects are empty, this method treats the
    // texture as unchanged, allowing DWM to skip redrawing.
    STDMETHOD(SetTexture)(THIS_
        _In_ IDCompositionTexture* pTexture,
        _In_count_(rectCount) const D2D_RECT_L *pRects,
        _In_ size_t rectCount) PURE;
};

//+-----------------------------------------------------------------------------
//
//  Interface:
//      PREVIEW_IDCompositionDevice5
//
//  Synopsis:
//      Serves as the root factory for composition dynamic textures
//
//------------------------------------------------------------------------------
#undef INTERFACE
#define INTERFACE PREVIEW_IDCompositionDevice5
DECLARE_INTERFACE_IID_(PREVIEW_IDCompositionDevice5, IDCompositionDevice4, "2C6BEBFE-A603-472F-AF34-D2443356E61B")
{
    STDMETHOD(CreateDynamicTexture)(THIS_
        _Outptr_ PREVIEW_IDCompositionDynamicTexture** compositionDynamicTexture) PURE;
};

#endif // #if (NTDDI_VERSION >= NTDDI_WIN11_GE)
