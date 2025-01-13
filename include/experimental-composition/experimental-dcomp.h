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
//      EXPERIMENTAL_IDCompositionDynamicTexture
//
//  Synopsis:
//      An interface representing a dynamically changing texture that can be bound to a
//      dcomp visual as a content.
//
//------------------------------------------------------------------------------
#undef INTERFACE
#define INTERFACE EXPERIMENTAL_IDCompositionDynamicTexture
DECLARE_INTERFACE_IID_(EXPERIMENTAL_IDCompositionDynamicTexture, IUnknown, "9575B228-D8A9-4F1E-A35B-C8D585C9CFAC")
{
    // Set current texture, assuming that every pixel has changed.
    STDMETHOD(SetTexture)(THIS_
        _In_ IDCompositionTexture* pContent) PURE;

    // Set current texture, assuming that only pixels inside provided rects has changed.
    // If provided with an empty array behaves like SetTexture(IDCompositionTexture*) above.
    STDMETHOD(SetTexture)(THIS_
        _In_ IDCompositionTexture* pContent,
        _In_ size_t rectCount,
        _In_count_(rectCount) const D2D_RECT_L *pRects) PURE;
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
DECLARE_INTERFACE_IID_(EXPERIMENTAL_IDCompositionDevice5, IDCompositionDevice4, "1F09CCEE-53AD-4E83-84AD-A6C771C5AE32")
{
    STDMETHOD(CreateDynamicTexture)(THIS_
        _Outptr_ EXPERIMENTAL_IDCompositionDynamicTexture** compositionDynamicTexture) PURE;
};

#endif // #if (NTDDI_VERSION >= NTDDI_WIN10_GE)
