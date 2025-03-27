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

#if (NTDDI_VERSION >= NTDDI_WIN11_GE)

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
        _In_ IDCompositionTexture* pContent) PURE;

    // Set current texture, assuming that only pixels inside provided rects has changed.
    // If provided with an empty array behaves like SetTexture(IDCompositionTexture*) above.
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

//+-----------------------------------------------------------------------------
//
//  Interface:
//      EXPERIMENTAL_IDCompositionDevice6
//
//  Synopsis:
//      An extension of composition device interface that enables support of d3d12 for composition textures.
//
//------------------------------------------------------------------------------
#undef INTERFACE
#define INTERFACE EXPERIMENTAL_IDCompositionDevice6
DECLARE_INTERFACE_IID_(EXPERIMENTAL_IDCompositionDevice6, EXPERIMENTAL_IDCompositionDevice5, "4CA97A18-CBFD-4B0D-89E1-F7FA86D8D63E")
{
    // Schedules an internal present for composition textures.
    // Each time you use SetVisual or SetTexture with composition texture that has d3d12 as underlying texture
    // you should call this method before the Commit().
    // You should provide an array of ID3D12CommandQueue objects, but not more than one queue per d3d device.
    // If rendering commands that affect composition textures were performed on a different queues of the same device
    // they all should be synchronized (for example by using fence) to the queue that is passed to this method.
    STDMETHOD(PresentCompositionTextures)(
        _In_reads_(queueCount) IUnknown* const* pCommandQueue,
        _In_ UINT queueCount) PURE;
};

#endif // #if (NTDDI_VERSION >= NTDDI_WIN11_GE)
