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
#ifndef __EXPERIMENTAL_COMPOSITION_EXPERIMENTAL_DCOMP_H__
#define __EXPERIMENTAL_COMPOSITION_EXPERIMENTAL_DCOMP_H__

#if (NTDDI_VERSION >= NTDDI_WIN10_GE)

//+-----------------------------------------------------------------------------
//
//  Interface:
//      EXPERIMENTAL_IDCompositionDynamicTexture
//
//  Synopsis:
//      An interface representing a dynamically changing texture that can be bound to a
//      dcomp visual as a content.
//
//------------------------------------------------------------------------------
#undef INTERFACE
#define INTERFACE EXPERIMENTAL_IDCompositionDynamicTexture
DECLARE_INTERFACE_IID_(EXPERIMENTAL_IDCompositionDynamicTexture, IUnknown, "A1DE1D3F-6405-447F-8E95-1383A34B0277")
{
    // Set current texture, assuming that every pixel has changed.
    STDMETHOD(SetTexture)(THIS_
        _In_ IDCompositionTexture* pTexture) PURE;

    // Set current texture, assuming that only pixels inside the provided rects have changed.
    //
    // DWM will use the provided rects to optimize the redrawing of the texture on the screen,
    // but it can't check the correctness of the provided rects, so the caller is responsible for
    // including every pixel that changed in at least one rect.
    // If provided with an empty array or empty rects, this method will treat the texture as identical
    // to the previous one so that DWM may choose not to redraw it.
    STDMETHOD(SetTexture)(THIS_
        _In_ IDCompositionTexture* pTexture,
        _In_count_(rectCount) const D2D_RECT_L *pRects,
        _In_ size_t rectCount) PURE;
};

//+-----------------------------------------------------------------------------
//
//  Interface:
//      EXPERIMENTAL_IDCompositionDevice5
//
//  Synopsis:
//      An extension of composition device interface that allows creating EXPERIMENTAL_IDCompositionDynamicTexture.
//
//------------------------------------------------------------------------------
#undef INTERFACE
#define INTERFACE EXPERIMENTAL_IDCompositionDevice5
DECLARE_INTERFACE_IID_(EXPERIMENTAL_IDCompositionDevice5, IDCompositionDevice4, "2C6BEBFE-A603-472F-AF34-D2443356E61B")
{
    STDMETHOD(CreateDynamicTexture)(THIS_
        _Outptr_ EXPERIMENTAL_IDCompositionDynamicTexture** compositionDynamicTexture) PURE;
};

#endif // #if (NTDDI_VERSION >= NTDDI_WIN10_GE)
#endif // #ifndef __EXPERIMENTAL_COMPOSITION_EXPERIMENTAL_DCOMP_H__
