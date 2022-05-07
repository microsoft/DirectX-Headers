/*** Autogenerated by WIDL 7.7 from d3d12compatibility.idl - Do not edit ***/

#ifdef _WIN32
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif
#include <rpc.h>
#include <rpcndr.h>
#endif

#ifndef COM_NO_WINDOWS_H
#include <windows.h>
#include <ole2.h>
#endif

#ifndef __d3d12compatibility_gnu_h__
#define __d3d12compatibility_gnu_h__

/* Forward declarations */

#ifndef __ID3D12CompatibilityDevice_FWD_DEFINED__
#define __ID3D12CompatibilityDevice_FWD_DEFINED__
typedef interface ID3D12CompatibilityDevice ID3D12CompatibilityDevice;
#ifdef __cplusplus
interface ID3D12CompatibilityDevice;
#endif /* __cplusplus */
#endif

#ifndef __D3D11On12CreatorID_FWD_DEFINED__
#define __D3D11On12CreatorID_FWD_DEFINED__
typedef interface D3D11On12CreatorID D3D11On12CreatorID;
#ifdef __cplusplus
interface D3D11On12CreatorID;
#endif /* __cplusplus */
#endif

#ifndef __D3D9On12CreatorID_FWD_DEFINED__
#define __D3D9On12CreatorID_FWD_DEFINED__
typedef interface D3D9On12CreatorID D3D9On12CreatorID;
#ifdef __cplusplus
interface D3D9On12CreatorID;
#endif /* __cplusplus */
#endif

#ifndef __OpenGLOn12CreatorID_FWD_DEFINED__
#define __OpenGLOn12CreatorID_FWD_DEFINED__
typedef interface OpenGLOn12CreatorID OpenGLOn12CreatorID;
#ifdef __cplusplus
interface OpenGLOn12CreatorID;
#endif /* __cplusplus */
#endif

#ifndef __OpenCLOn12CreatorID_FWD_DEFINED__
#define __OpenCLOn12CreatorID_FWD_DEFINED__
typedef interface OpenCLOn12CreatorID OpenCLOn12CreatorID;
#ifdef __cplusplus
interface OpenCLOn12CreatorID;
#endif /* __cplusplus */
#endif

#ifndef __DirectMLTensorFlowCreatorID_FWD_DEFINED__
#define __DirectMLTensorFlowCreatorID_FWD_DEFINED__
typedef interface DirectMLTensorFlowCreatorID DirectMLTensorFlowCreatorID;
#ifdef __cplusplus
interface DirectMLTensorFlowCreatorID;
#endif /* __cplusplus */
#endif

/* Headers for imported files */

#include <oaidl.h>
#include <ocidl.h>
#include <d3d11on12.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __ID3D12LifetimeTracker_FWD_DEFINED__
#define __ID3D12LifetimeTracker_FWD_DEFINED__
typedef interface ID3D12LifetimeTracker ID3D12LifetimeTracker;
#ifdef __cplusplus
interface ID3D12LifetimeTracker;
#endif /* __cplusplus */
#endif

#ifndef __ID3D12SwapChainAssistant_FWD_DEFINED__
#define __ID3D12SwapChainAssistant_FWD_DEFINED__
typedef interface ID3D12SwapChainAssistant ID3D12SwapChainAssistant;
#ifdef __cplusplus
interface ID3D12SwapChainAssistant;
#endif /* __cplusplus */
#endif

#include <winapifamily.h>
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_GAMES)
typedef enum D3D12_COMPATIBILITY_SHARED_FLAGS {
    D3D12_COMPATIBILITY_SHARED_FLAG_NONE = 0,
    D3D12_COMPATIBILITY_SHARED_FLAG_NON_NT_HANDLE = 0x1,
    D3D12_COMPATIBILITY_SHARED_FLAG_KEYED_MUTEX = 0x2,
    D3D12_COMPATIBILITY_SHARED_FLAG_9_ON_12 = 0x4
} D3D12_COMPATIBILITY_SHARED_FLAGS;
DEFINE_ENUM_FLAG_OPERATORS( D3D12_COMPATIBILITY_SHARED_FLAGS );
typedef enum D3D12_REFLECT_SHARED_PROPERTY {
    D3D12_REFLECT_SHARED_PROPERTY_D3D11_RESOURCE_FLAGS = 0,
    D3D12_REFELCT_SHARED_PROPERTY_COMPATIBILITY_SHARED_FLAGS = 1,
    D3D12_REFLECT_SHARED_PROPERTY_NON_NT_SHARED_HANDLE = 2
} D3D12_REFLECT_SHARED_PROPERTY;
/*****************************************************************************
 * ID3D12CompatibilityDevice interface
 */
#ifndef __ID3D12CompatibilityDevice_INTERFACE_DEFINED__
#define __ID3D12CompatibilityDevice_INTERFACE_DEFINED__

DEFINE_GUID(IID_ID3D12CompatibilityDevice, 0x8f1c0e3c, 0xfae3, 0x4a82, 0xb0,0x98, 0xbf,0xe1,0x70,0x82,0x07,0xff);
#if defined(__cplusplus) && !defined(CINTERFACE)
MIDL_INTERFACE("8f1c0e3c-fae3-4a82-b098-bfe1708207ff")
ID3D12CompatibilityDevice : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE CreateSharedResource(
        const D3D12_HEAP_PROPERTIES *pHeapProperties,
        D3D12_HEAP_FLAGS HeapFlags,
        const D3D12_RESOURCE_DESC *pDesc,
        D3D12_RESOURCE_STATES InitialResourceState,
        const D3D12_CLEAR_VALUE *pOptimizedClearValue,
        const D3D11_RESOURCE_FLAGS *pFlags11,
        D3D12_COMPATIBILITY_SHARED_FLAGS CompatibilityFlags,
        ID3D12LifetimeTracker *pLifetimeTracker,
        ID3D12SwapChainAssistant *pOwningSwapchain,
        REFIID riid,
        void **ppResource) = 0;

    virtual HRESULT STDMETHODCALLTYPE CreateSharedHeap(
        const D3D12_HEAP_DESC *pHeapDesc,
        D3D12_COMPATIBILITY_SHARED_FLAGS CompatibilityFlags,
        REFIID riid,
        void **ppHeap) = 0;

    virtual HRESULT STDMETHODCALLTYPE ReflectSharedProperties(
        ID3D12Object *pHeapOrResource,
        D3D12_REFLECT_SHARED_PROPERTY ReflectType,
        void *pData,
        UINT DataSize) = 0;

};
#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL(ID3D12CompatibilityDevice, 0x8f1c0e3c, 0xfae3, 0x4a82, 0xb0,0x98, 0xbf,0xe1,0x70,0x82,0x07,0xff)
#endif
#else
typedef struct ID3D12CompatibilityDeviceVtbl {
    BEGIN_INTERFACE

    /*** IUnknown methods ***/
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        ID3D12CompatibilityDevice *This,
        REFIID riid,
        void **ppvObject);

    ULONG (STDMETHODCALLTYPE *AddRef)(
        ID3D12CompatibilityDevice *This);

    ULONG (STDMETHODCALLTYPE *Release)(
        ID3D12CompatibilityDevice *This);

    /*** ID3D12CompatibilityDevice methods ***/
    HRESULT (STDMETHODCALLTYPE *CreateSharedResource)(
        ID3D12CompatibilityDevice *This,
        const D3D12_HEAP_PROPERTIES *pHeapProperties,
        D3D12_HEAP_FLAGS HeapFlags,
        const D3D12_RESOURCE_DESC *pDesc,
        D3D12_RESOURCE_STATES InitialResourceState,
        const D3D12_CLEAR_VALUE *pOptimizedClearValue,
        const D3D11_RESOURCE_FLAGS *pFlags11,
        D3D12_COMPATIBILITY_SHARED_FLAGS CompatibilityFlags,
        ID3D12LifetimeTracker *pLifetimeTracker,
        ID3D12SwapChainAssistant *pOwningSwapchain,
        REFIID riid,
        void **ppResource);

    HRESULT (STDMETHODCALLTYPE *CreateSharedHeap)(
        ID3D12CompatibilityDevice *This,
        const D3D12_HEAP_DESC *pHeapDesc,
        D3D12_COMPATIBILITY_SHARED_FLAGS CompatibilityFlags,
        REFIID riid,
        void **ppHeap);

    HRESULT (STDMETHODCALLTYPE *ReflectSharedProperties)(
        ID3D12CompatibilityDevice *This,
        ID3D12Object *pHeapOrResource,
        D3D12_REFLECT_SHARED_PROPERTY ReflectType,
        void *pData,
        UINT DataSize);

    END_INTERFACE
} ID3D12CompatibilityDeviceVtbl;

interface ID3D12CompatibilityDevice {
    CONST_VTBL ID3D12CompatibilityDeviceVtbl* lpVtbl;
};

#ifdef COBJMACROS
#ifndef WIDL_C_INLINE_WRAPPERS
/*** IUnknown methods ***/
#define ID3D12CompatibilityDevice_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define ID3D12CompatibilityDevice_AddRef(This) (This)->lpVtbl->AddRef(This)
#define ID3D12CompatibilityDevice_Release(This) (This)->lpVtbl->Release(This)
/*** ID3D12CompatibilityDevice methods ***/
#define ID3D12CompatibilityDevice_CreateSharedResource(This,pHeapProperties,HeapFlags,pDesc,InitialResourceState,pOptimizedClearValue,pFlags11,CompatibilityFlags,pLifetimeTracker,pOwningSwapchain,riid,ppResource) (This)->lpVtbl->CreateSharedResource(This,pHeapProperties,HeapFlags,pDesc,InitialResourceState,pOptimizedClearValue,pFlags11,CompatibilityFlags,pLifetimeTracker,pOwningSwapchain,riid,ppResource)
#define ID3D12CompatibilityDevice_CreateSharedHeap(This,pHeapDesc,CompatibilityFlags,riid,ppHeap) (This)->lpVtbl->CreateSharedHeap(This,pHeapDesc,CompatibilityFlags,riid,ppHeap)
#define ID3D12CompatibilityDevice_ReflectSharedProperties(This,pHeapOrResource,ReflectType,pData,DataSize) (This)->lpVtbl->ReflectSharedProperties(This,pHeapOrResource,ReflectType,pData,DataSize)
#else
/*** IUnknown methods ***/
static FORCEINLINE HRESULT ID3D12CompatibilityDevice_QueryInterface(ID3D12CompatibilityDevice* This,REFIID riid,void **ppvObject) {
    return This->lpVtbl->QueryInterface(This,riid,ppvObject);
}
static FORCEINLINE ULONG ID3D12CompatibilityDevice_AddRef(ID3D12CompatibilityDevice* This) {
    return This->lpVtbl->AddRef(This);
}
static FORCEINLINE ULONG ID3D12CompatibilityDevice_Release(ID3D12CompatibilityDevice* This) {
    return This->lpVtbl->Release(This);
}
/*** ID3D12CompatibilityDevice methods ***/
static FORCEINLINE HRESULT ID3D12CompatibilityDevice_CreateSharedResource(ID3D12CompatibilityDevice* This,const D3D12_HEAP_PROPERTIES *pHeapProperties,D3D12_HEAP_FLAGS HeapFlags,const D3D12_RESOURCE_DESC *pDesc,D3D12_RESOURCE_STATES InitialResourceState,const D3D12_CLEAR_VALUE *pOptimizedClearValue,const D3D11_RESOURCE_FLAGS *pFlags11,D3D12_COMPATIBILITY_SHARED_FLAGS CompatibilityFlags,ID3D12LifetimeTracker *pLifetimeTracker,ID3D12SwapChainAssistant *pOwningSwapchain,REFIID riid,void **ppResource) {
    return This->lpVtbl->CreateSharedResource(This,pHeapProperties,HeapFlags,pDesc,InitialResourceState,pOptimizedClearValue,pFlags11,CompatibilityFlags,pLifetimeTracker,pOwningSwapchain,riid,ppResource);
}
static FORCEINLINE HRESULT ID3D12CompatibilityDevice_CreateSharedHeap(ID3D12CompatibilityDevice* This,const D3D12_HEAP_DESC *pHeapDesc,D3D12_COMPATIBILITY_SHARED_FLAGS CompatibilityFlags,REFIID riid,void **ppHeap) {
    return This->lpVtbl->CreateSharedHeap(This,pHeapDesc,CompatibilityFlags,riid,ppHeap);
}
static FORCEINLINE HRESULT ID3D12CompatibilityDevice_ReflectSharedProperties(ID3D12CompatibilityDevice* This,ID3D12Object *pHeapOrResource,D3D12_REFLECT_SHARED_PROPERTY ReflectType,void *pData,UINT DataSize) {
    return This->lpVtbl->ReflectSharedProperties(This,pHeapOrResource,ReflectType,pData,DataSize);
}
#endif
#endif

#endif


#endif  /* __ID3D12CompatibilityDevice_INTERFACE_DEFINED__ */

/*****************************************************************************
 * D3D11On12CreatorID interface
 */
#ifndef __D3D11On12CreatorID_INTERFACE_DEFINED__
#define __D3D11On12CreatorID_INTERFACE_DEFINED__

DEFINE_GUID(IID_D3D11On12CreatorID, 0xedbf5678, 0x2960, 0x4e81, 0x84,0x29, 0x99,0xd4,0xb2,0x63,0x0c,0x4e);
#if defined(__cplusplus) && !defined(CINTERFACE)
MIDL_INTERFACE("edbf5678-2960-4e81-8429-99d4b2630c4e")
D3D11On12CreatorID : public IUnknown
{
};
#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL(D3D11On12CreatorID, 0xedbf5678, 0x2960, 0x4e81, 0x84,0x29, 0x99,0xd4,0xb2,0x63,0x0c,0x4e)
#endif
#else
typedef struct D3D11On12CreatorIDVtbl {
    BEGIN_INTERFACE

    /*** IUnknown methods ***/
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        D3D11On12CreatorID *This,
        REFIID riid,
        void **ppvObject);

    ULONG (STDMETHODCALLTYPE *AddRef)(
        D3D11On12CreatorID *This);

    ULONG (STDMETHODCALLTYPE *Release)(
        D3D11On12CreatorID *This);

    END_INTERFACE
} D3D11On12CreatorIDVtbl;

interface D3D11On12CreatorID {
    CONST_VTBL D3D11On12CreatorIDVtbl* lpVtbl;
};

#ifdef COBJMACROS
#ifndef WIDL_C_INLINE_WRAPPERS
/*** IUnknown methods ***/
#define D3D11On12CreatorID_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define D3D11On12CreatorID_AddRef(This) (This)->lpVtbl->AddRef(This)
#define D3D11On12CreatorID_Release(This) (This)->lpVtbl->Release(This)
#else
/*** IUnknown methods ***/
static FORCEINLINE HRESULT D3D11On12CreatorID_QueryInterface(D3D11On12CreatorID* This,REFIID riid,void **ppvObject) {
    return This->lpVtbl->QueryInterface(This,riid,ppvObject);
}
static FORCEINLINE ULONG D3D11On12CreatorID_AddRef(D3D11On12CreatorID* This) {
    return This->lpVtbl->AddRef(This);
}
static FORCEINLINE ULONG D3D11On12CreatorID_Release(D3D11On12CreatorID* This) {
    return This->lpVtbl->Release(This);
}
#endif
#endif

#endif


#endif  /* __D3D11On12CreatorID_INTERFACE_DEFINED__ */

/*****************************************************************************
 * D3D9On12CreatorID interface
 */
#ifndef __D3D9On12CreatorID_INTERFACE_DEFINED__
#define __D3D9On12CreatorID_INTERFACE_DEFINED__

DEFINE_GUID(IID_D3D9On12CreatorID, 0xfffcbb7f, 0x15d3, 0x42a2, 0x84,0x1e, 0x9d,0x8d,0x32,0xf3,0x7d,0xdd);
#if defined(__cplusplus) && !defined(CINTERFACE)
MIDL_INTERFACE("fffcbb7f-15d3-42a2-841e-9d8d32f37ddd")
D3D9On12CreatorID : public IUnknown
{
};
#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL(D3D9On12CreatorID, 0xfffcbb7f, 0x15d3, 0x42a2, 0x84,0x1e, 0x9d,0x8d,0x32,0xf3,0x7d,0xdd)
#endif
#else
typedef struct D3D9On12CreatorIDVtbl {
    BEGIN_INTERFACE

    /*** IUnknown methods ***/
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        D3D9On12CreatorID *This,
        REFIID riid,
        void **ppvObject);

    ULONG (STDMETHODCALLTYPE *AddRef)(
        D3D9On12CreatorID *This);

    ULONG (STDMETHODCALLTYPE *Release)(
        D3D9On12CreatorID *This);

    END_INTERFACE
} D3D9On12CreatorIDVtbl;

interface D3D9On12CreatorID {
    CONST_VTBL D3D9On12CreatorIDVtbl* lpVtbl;
};

#ifdef COBJMACROS
#ifndef WIDL_C_INLINE_WRAPPERS
/*** IUnknown methods ***/
#define D3D9On12CreatorID_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define D3D9On12CreatorID_AddRef(This) (This)->lpVtbl->AddRef(This)
#define D3D9On12CreatorID_Release(This) (This)->lpVtbl->Release(This)
#else
/*** IUnknown methods ***/
static FORCEINLINE HRESULT D3D9On12CreatorID_QueryInterface(D3D9On12CreatorID* This,REFIID riid,void **ppvObject) {
    return This->lpVtbl->QueryInterface(This,riid,ppvObject);
}
static FORCEINLINE ULONG D3D9On12CreatorID_AddRef(D3D9On12CreatorID* This) {
    return This->lpVtbl->AddRef(This);
}
static FORCEINLINE ULONG D3D9On12CreatorID_Release(D3D9On12CreatorID* This) {
    return This->lpVtbl->Release(This);
}
#endif
#endif

#endif


#endif  /* __D3D9On12CreatorID_INTERFACE_DEFINED__ */

/*****************************************************************************
 * OpenGLOn12CreatorID interface
 */
#ifndef __OpenGLOn12CreatorID_INTERFACE_DEFINED__
#define __OpenGLOn12CreatorID_INTERFACE_DEFINED__

DEFINE_GUID(IID_OpenGLOn12CreatorID, 0x6bb3cd34, 0x0d19, 0x45ab, 0x97,0xed, 0xd7,0x20,0xba,0x3d,0xfc,0x80);
#if defined(__cplusplus) && !defined(CINTERFACE)
MIDL_INTERFACE("6bb3cd34-0d19-45ab-97ed-d720ba3dfc80")
OpenGLOn12CreatorID : public IUnknown
{
};
#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL(OpenGLOn12CreatorID, 0x6bb3cd34, 0x0d19, 0x45ab, 0x97,0xed, 0xd7,0x20,0xba,0x3d,0xfc,0x80)
#endif
#else
typedef struct OpenGLOn12CreatorIDVtbl {
    BEGIN_INTERFACE

    /*** IUnknown methods ***/
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        OpenGLOn12CreatorID *This,
        REFIID riid,
        void **ppvObject);

    ULONG (STDMETHODCALLTYPE *AddRef)(
        OpenGLOn12CreatorID *This);

    ULONG (STDMETHODCALLTYPE *Release)(
        OpenGLOn12CreatorID *This);

    END_INTERFACE
} OpenGLOn12CreatorIDVtbl;

interface OpenGLOn12CreatorID {
    CONST_VTBL OpenGLOn12CreatorIDVtbl* lpVtbl;
};

#ifdef COBJMACROS
#ifndef WIDL_C_INLINE_WRAPPERS
/*** IUnknown methods ***/
#define OpenGLOn12CreatorID_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define OpenGLOn12CreatorID_AddRef(This) (This)->lpVtbl->AddRef(This)
#define OpenGLOn12CreatorID_Release(This) (This)->lpVtbl->Release(This)
#else
/*** IUnknown methods ***/
static FORCEINLINE HRESULT OpenGLOn12CreatorID_QueryInterface(OpenGLOn12CreatorID* This,REFIID riid,void **ppvObject) {
    return This->lpVtbl->QueryInterface(This,riid,ppvObject);
}
static FORCEINLINE ULONG OpenGLOn12CreatorID_AddRef(OpenGLOn12CreatorID* This) {
    return This->lpVtbl->AddRef(This);
}
static FORCEINLINE ULONG OpenGLOn12CreatorID_Release(OpenGLOn12CreatorID* This) {
    return This->lpVtbl->Release(This);
}
#endif
#endif

#endif


#endif  /* __OpenGLOn12CreatorID_INTERFACE_DEFINED__ */

/*****************************************************************************
 * OpenCLOn12CreatorID interface
 */
#ifndef __OpenCLOn12CreatorID_INTERFACE_DEFINED__
#define __OpenCLOn12CreatorID_INTERFACE_DEFINED__

DEFINE_GUID(IID_OpenCLOn12CreatorID, 0x3f76bb74, 0x91b5, 0x4a88, 0xb1,0x26, 0x20,0xca,0x03,0x31,0xcd,0x60);
#if defined(__cplusplus) && !defined(CINTERFACE)
MIDL_INTERFACE("3f76bb74-91b5-4a88-b126-20ca0331cd60")
OpenCLOn12CreatorID : public IUnknown
{
};
#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL(OpenCLOn12CreatorID, 0x3f76bb74, 0x91b5, 0x4a88, 0xb1,0x26, 0x20,0xca,0x03,0x31,0xcd,0x60)
#endif
#else
typedef struct OpenCLOn12CreatorIDVtbl {
    BEGIN_INTERFACE

    /*** IUnknown methods ***/
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        OpenCLOn12CreatorID *This,
        REFIID riid,
        void **ppvObject);

    ULONG (STDMETHODCALLTYPE *AddRef)(
        OpenCLOn12CreatorID *This);

    ULONG (STDMETHODCALLTYPE *Release)(
        OpenCLOn12CreatorID *This);

    END_INTERFACE
} OpenCLOn12CreatorIDVtbl;

interface OpenCLOn12CreatorID {
    CONST_VTBL OpenCLOn12CreatorIDVtbl* lpVtbl;
};

#ifdef COBJMACROS
#ifndef WIDL_C_INLINE_WRAPPERS
/*** IUnknown methods ***/
#define OpenCLOn12CreatorID_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define OpenCLOn12CreatorID_AddRef(This) (This)->lpVtbl->AddRef(This)
#define OpenCLOn12CreatorID_Release(This) (This)->lpVtbl->Release(This)
#else
/*** IUnknown methods ***/
static FORCEINLINE HRESULT OpenCLOn12CreatorID_QueryInterface(OpenCLOn12CreatorID* This,REFIID riid,void **ppvObject) {
    return This->lpVtbl->QueryInterface(This,riid,ppvObject);
}
static FORCEINLINE ULONG OpenCLOn12CreatorID_AddRef(OpenCLOn12CreatorID* This) {
    return This->lpVtbl->AddRef(This);
}
static FORCEINLINE ULONG OpenCLOn12CreatorID_Release(OpenCLOn12CreatorID* This) {
    return This->lpVtbl->Release(This);
}
#endif
#endif

#endif


#endif  /* __OpenCLOn12CreatorID_INTERFACE_DEFINED__ */

/*****************************************************************************
 * DirectMLTensorFlowCreatorID interface
 */
#ifndef __DirectMLTensorFlowCreatorID_INTERFACE_DEFINED__
#define __DirectMLTensorFlowCreatorID_INTERFACE_DEFINED__

DEFINE_GUID(IID_DirectMLTensorFlowCreatorID, 0xcb7490ac, 0x8a0f, 0x44ec, 0x9b,0x7b, 0x6f,0x4c,0xaf,0xe8,0xe9,0xab);
#if defined(__cplusplus) && !defined(CINTERFACE)
MIDL_INTERFACE("cb7490ac-8a0f-44ec-9b7b-6f4cafe8e9ab")
DirectMLTensorFlowCreatorID : public IUnknown
{
};
#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL(DirectMLTensorFlowCreatorID, 0xcb7490ac, 0x8a0f, 0x44ec, 0x9b,0x7b, 0x6f,0x4c,0xaf,0xe8,0xe9,0xab)
#endif
#else
typedef struct DirectMLTensorFlowCreatorIDVtbl {
    BEGIN_INTERFACE

    /*** IUnknown methods ***/
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        DirectMLTensorFlowCreatorID *This,
        REFIID riid,
        void **ppvObject);

    ULONG (STDMETHODCALLTYPE *AddRef)(
        DirectMLTensorFlowCreatorID *This);

    ULONG (STDMETHODCALLTYPE *Release)(
        DirectMLTensorFlowCreatorID *This);

    END_INTERFACE
} DirectMLTensorFlowCreatorIDVtbl;

interface DirectMLTensorFlowCreatorID {
    CONST_VTBL DirectMLTensorFlowCreatorIDVtbl* lpVtbl;
};

#ifdef COBJMACROS
#ifndef WIDL_C_INLINE_WRAPPERS
/*** IUnknown methods ***/
#define DirectMLTensorFlowCreatorID_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define DirectMLTensorFlowCreatorID_AddRef(This) (This)->lpVtbl->AddRef(This)
#define DirectMLTensorFlowCreatorID_Release(This) (This)->lpVtbl->Release(This)
#else
/*** IUnknown methods ***/
static FORCEINLINE HRESULT DirectMLTensorFlowCreatorID_QueryInterface(DirectMLTensorFlowCreatorID* This,REFIID riid,void **ppvObject) {
    return This->lpVtbl->QueryInterface(This,riid,ppvObject);
}
static FORCEINLINE ULONG DirectMLTensorFlowCreatorID_AddRef(DirectMLTensorFlowCreatorID* This) {
    return This->lpVtbl->AddRef(This);
}
static FORCEINLINE ULONG DirectMLTensorFlowCreatorID_Release(DirectMLTensorFlowCreatorID* This) {
    return This->lpVtbl->Release(This);
}
#endif
#endif

#endif


#endif  /* __DirectMLTensorFlowCreatorID_INTERFACE_DEFINED__ */

#endif /* WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_GAMES) */
DEFINE_GUID(IID_ID3D12CompatibilityDevice,0x8f1c0e3c,0xfae3,0x4a82,0xb0,0x98,0xbf,0xe1,0x70,0x82,0x07,0xff);
DEFINE_GUID(IID_D3D11On12CreatorID,0xedbf5678,0x2960,0x4e81,0x84,0x29,0x99,0xd4,0xb2,0x63,0x0c,0x4e);
DEFINE_GUID(IID_D3D9On12CreatorID,0xfffcbb7f,0x15d3,0x42a2,0x84,0x1e,0x9d,0x8d,0x32,0xf3,0x7d,0xdd);
DEFINE_GUID(IID_OpenGLOn12CreatorID,0x6bb3cd34,0x0d19,0x45ab,0x97,0xed,0xd7,0x20,0xba,0x3d,0xfc,0x80);
DEFINE_GUID(IID_OpenCLOn12CreatorID,0x3f76bb74,0x91b5,0x4a88,0xb1,0x26,0x20,0xca,0x03,0x31,0xcd,0x60);
DEFINE_GUID(IID_DirectMLTensorFlowCreatorID,0xcb7490ac,0x8a0f,0x44ec,0x9b,0x7b,0x6f,0x4c,0xaf,0xe8,0xe9,0xab);
/* Begin additional prototypes for all interfaces */


/* End additional prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __d3d12compatibility_gnu_h__ */