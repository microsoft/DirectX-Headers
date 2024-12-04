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
//  Interface:
//      EXPERIMENTAL_IPresentationManagerSync
//
//  Synopsis:
//      An interface to allow a presentation manager to engage in VRR
//      presentation
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
//      EXPERIMENTAL_IPresentationManagerContextlessPresent
//
//  Synopsis:
//      An interface to allow a presentation manager to engage in D3D12,
//      contexless presentation
//
//------------------------------------------------------------------------------
#undef INTERFACE
#define INTERFACE EXPERIMENTAL_IPresentationManagerContextlessPresent
DECLARE_INTERFACE_IID_(EXPERIMENTAL_IPresentationManagerContextlessPresent, IUnknown, "49967145-083F-47F2-A4A0-F9FDE65DA523")
{
    STDMETHOD(PreparePresent)(THIS_
        _In_ IUnknown *commandQueue,
        _In_ IUnknown *commandList) PURE;

    STDMETHOD(Present)(THIS_
        _In_ IUnknown *fence,
        _In_ UINT64 fenceValue) PURE;
};

#endif // #if (NTDDI_VERSION >= NTDDI_WIN10_GE)
