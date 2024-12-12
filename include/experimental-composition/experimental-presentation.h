/*-------------------------------------------------------------------------------------
 *
 * Copyright (c) Microsoft Corporation
 * Licensed under the MIT license
 *
 * This file defines experimental Presentation API interfaces. Anything found in this
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
//  Constant:
//      c_variableRefreshDuration
//
//  Synopsis:
//      An SystemInterruptTime that can be passed into
//      IPresentationManager::SetPreferredPresentDuration to allow variable
//      refresh rate to engage for eligible periods of presents
//
//------------------------------------------------------------------------------
static constexpr SystemInterruptTime c_variableRefreshDuration = { UINT64_MAX };

//+-----------------------------------------------------------------------------
//
//  Interface:
//      EXPERIMENTAL_IPresentationManagerSync
//
//  Synopsis:
//      An interface to allow a presentation manager presents to tear (flip
//      outside of vblank)
//
//------------------------------------------------------------------------------
#undef INTERFACE
#define INTERFACE EXPERIMENTAL_IPresentationManagerSync
DECLARE_INTERFACE_IID_(EXPERIMENTAL_IPresentationManagerSync, IUnknown, "685BBA49-9559-4F02-A720-9C9B460A7CC7")
{
    STDMETHOD(AllowTearing)(THIS_ 
        _In_ boolean tearing) PURE;
};

//+-----------------------------------------------------------------------------
//
//  Interface:
//      EXPERIMENTAL_IPresentationManagerDX12
//
//  Synopsis:
//      Supports D3D12-based presentation of presentation manager
//
//------------------------------------------------------------------------------
#undef INTERFACE
#define INTERFACE EXPERIMENTAL_IPresentationManagerDX12
DECLARE_INTERFACE_IID_(EXPERIMENTAL_IPresentationManagerDX12, IUnknown, "B661B85A-B4F4-485C-865A-0389AD79C06D")
{
    STDMETHOD(ExecuteCommandListsAndPresent)(THIS_
        _In_ ID3D12CommandQueue* commandQueue,
        _In_ UINT commandListCount,
        _In_reads_(commandListCount) ID3D12CommandList* const* commandLists) PURE;
};

#endif // #if (NTDDI_VERSION >= NTDDI_WIN10_GE)
