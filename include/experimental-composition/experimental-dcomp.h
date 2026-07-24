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
#pragma once

//+-----------------------------------------------------------------------------
//
//  Interface:
//      IDCompositionBrush
//
//  Synopsis:
//      Abstract base type for DComp brush content. A brush is content whose
//      render bounds are taken from the hosting visual's size rather than from
//      any intrinsic content size. Brushes can be attached to a visual via
//      IDCompositionVisual::SetContent.
//
//      This interface intentionally exposes no methods of its own; concrete
//      brush types extend it.
//
//------------------------------------------------------------------------------
#undef INTERFACE
#define INTERFACE IDCompositionBrush
DECLARE_INTERFACE_IID_(IDCompositionBrush, IUnknown, "A50CAF0E-C2D5-4285-B1C3-C7934A5907E6")
{
};

//+-----------------------------------------------------------------------------
//
//  Interface:
//      IDCompositionColorBrush
//
//  Synopsis:
//      A brush that fills the hosting visual's bounds with a single color.
//      Analogous to WinComp's CompositionColorBrush but attached to a DComp
//      visual via SetContent rather than to a sprite visual.
//
//------------------------------------------------------------------------------
#undef INTERFACE
#define INTERFACE IDCompositionColorBrush
DECLARE_INTERFACE_IID_(IDCompositionColorBrush, IDCompositionBrush, "74D54EE0-11B0-41CA-A26A-982234916F9E")
{
    // Updates the brush color.
    STDMETHOD(SetColor)(THIS_
        _In_ const D2D_COLOR_F& color) PURE;

    STDMETHOD(SetRed)(THIS_
        float red) PURE;

    STDMETHOD(SetRed)(THIS_
        _In_ IDCompositionAnimation* animation) PURE;

    STDMETHOD(SetGreen)(THIS_
        float green) PURE;

    STDMETHOD(SetGreen)(THIS_
        _In_ IDCompositionAnimation* animation) PURE;

    STDMETHOD(SetBlue)(THIS_
        float blue) PURE;

    STDMETHOD(SetBlue)(THIS_
        _In_ IDCompositionAnimation* animation) PURE;

    STDMETHOD(SetAlpha)(THIS_
        float alpha) PURE;

    STDMETHOD(SetAlpha)(THIS_
        _In_ IDCompositionAnimation* animation) PURE;
};

//+-----------------------------------------------------------------------------
//
//  Interface:
//      IDCompositionSurfaceBrush
//
//  Synopsis:
//      A brush that fills the hosting visual's layout bounds with the contents
//      of a surface.
//
//------------------------------------------------------------------------------
#undef INTERFACE
#define INTERFACE IDCompositionSurfaceBrush
DECLARE_INTERFACE_IID_(IDCompositionSurfaceBrush, IDCompositionBrush, "E5C2A09B-2C5A-4C98-9E0D-8FE8B2E1FE38")
{
    // Sets the surface that the brush draws. Pass nullptr to clear.
    STDMETHOD(SetSurface)(THIS_
        _In_opt_ IUnknown* surface) PURE;
};

//+-----------------------------------------------------------------------------
//
//  Interface:
//      EXPERIMENTAL_IDCompositionVisual4
//
//  Synopsis:
//      Adds Width/Height/RelativeOffset/RelativeSize property setters (scalar
//      and animation overloads) to the base IDCompositionVisual3 chain.
//
//------------------------------------------------------------------------------
#undef INTERFACE
#define INTERFACE EXPERIMENTAL_IDCompositionVisual4
DECLARE_INTERFACE_IID_(EXPERIMENTAL_IDCompositionVisual4, IDCompositionVisual3, "9B512ECD-B56E-4E09-A4A4-2F2E2AB5B45B")
{
    // Specifies the width for the visual.
    STDMETHOD(SetWidth)(THIS_
        _In_ float width) PURE;

    // Specifies the height for the visual.
    STDMETHOD(SetHeight)(THIS_
        _In_ float height) PURE;

    // Changes the value of the RelativeOffsetX property.
    STDMETHOD(SetRelativeOffsetX)(THIS_
        _In_ float offsetX) PURE;

    // Changes the value of the RelativeOffsetY property.
    STDMETHOD(SetRelativeOffsetY)(THIS_
        _In_ float offsetY) PURE;

    // Specifies the relative width for the visual.
    STDMETHOD(SetRelativeWidth)(THIS_
        _In_ float width) PURE;

    // Specifies the relative height for the visual.
    STDMETHOD(SetRelativeHeight)(THIS_
        _In_ float height) PURE;

    // Animates the value of the Width property.
    STDMETHOD(SetWidth)(THIS_
        _In_ IDCompositionAnimation* pAnimation) PURE;

    // Animates the value of the Height property.
    STDMETHOD(SetHeight)(THIS_
        _In_ IDCompositionAnimation* pAnimation) PURE;

    // Animates the value of the RelativeOffsetX property.
    STDMETHOD(SetRelativeOffsetX)(THIS_
        _In_ IDCompositionAnimation* pAnimation) PURE;

    // Animates the value of the RelativeOffsetY property.
    STDMETHOD(SetRelativeOffsetY)(THIS_
        _In_ IDCompositionAnimation* pAnimation) PURE;

    // Animates the value of the RelativeWidth property.
    STDMETHOD(SetRelativeWidth)(THIS_
        _In_ IDCompositionAnimation* pAnimation) PURE;

    // Animates the value of the RelativeHeight property.
    STDMETHOD(SetRelativeHeight)(THIS_
        _In_ IDCompositionAnimation* pAnimation) PURE;
};

#if (NTDDI_VERSION >= NTDDI_WIN11_GE)

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
DECLARE_INTERFACE_IID_(EXPERIMENTAL_IDCompositionDevice6, IDCompositionDevice5, "4CA97A18-CBFD-4B0D-89E1-F7FA86D8D63E")
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

//+-----------------------------------------------------------------------------
//
//  Interface:
//      EXPERIMENTAL_IDCompositionDevice7
//
//  Synopsis:
//      Adds factory entry points for the new brush content types.
//
//------------------------------------------------------------------------------
#undef INTERFACE
#define INTERFACE EXPERIMENTAL_IDCompositionDevice7
DECLARE_INTERFACE_IID_(EXPERIMENTAL_IDCompositionDevice7, EXPERIMENTAL_IDCompositionDevice6, "07B0BFD7-BBA5-4F37-8965-B9333275CA51")
{
    // Creates a solid color brush whose initial color is transparent black.
    STDMETHOD(CreateColorBrush)(THIS_
        _Outptr_ IDCompositionColorBrush** colorBrush) PURE;

    // Creates a surface brush whose initial surface is null.
    STDMETHOD(CreateSurfaceBrush)(THIS_
        _Outptr_ IDCompositionSurfaceBrush** surfaceBrush) PURE;
};

#endif // #if (NTDDI_VERSION >= NTDDI_WIN11_GE)
