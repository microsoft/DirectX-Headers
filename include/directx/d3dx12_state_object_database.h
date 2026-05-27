//*********************************************************
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License (MIT).
//
//*********************************************************

#pragma once

#ifndef __cplusplus
#error D3DX12 requires C++
#endif

#include "d3d12.h"

#if defined(D3D12_SDK_VERSION) && (D3D12_SDK_VERSION >= 620)
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <mutex>
#include <optional>
#ifndef D3DX12_COM_PTR
#ifndef D3DX12_USE_ATL
#include <wrl/client.h>
#define D3DX12_COM_PTR Microsoft::WRL::ComPtr
#define D3DX12_COM_PTR_GET(x) x.Get()
#define D3DX12_COM_PTR_ADDRESSOF(x) x.GetAddressOf()
#else
#include <atlbase.h>
#define D3DX12_COM_PTR ATL::CComPtr
#define D3DX12_COM_PTR_GET(x) x.p
#define D3DX12_COM_PTR_ADDRESSOF(x) &x.p
#endif
#endif
//------------------------------------------------------------------------------------------------
// No-op lock satisfying BasicLockable.  Use as the default TLock template
// parameter on map types that don't need thread safety.
//
struct CD3DX12_SINGLE_THREADED_LOCK
{
    void lock() noexcept {}
    void unlock() noexcept {}
};

//------------------------------------------------------------------------------------------------
// A generic map keyed by database key structs (D3D12_DATABASE_STATE_OBJECT_KEY, etc.).
// Stores COM pointers with automatic AddRef/Release via D3DX12_COM_PTR.
// T is the COM interface type (e.g. ID3D12RootSignature, not ID3D12RootSignature*).
// TLock controls thread safety: defaults to CD3DX12_SINGLE_THREADED_LOCK (no-op).
// Supply std::mutex (or any BasicLockable) for concurrent access.
//
template<typename T, typename TLock = CD3DX12_SINGLE_THREADED_LOCK>
struct CD3DX12_KEY_TO_OBJECT_MAP
{
    struct BlobKey
    {
        std::vector<BYTE> data;

        BlobKey() = default;
        BlobKey(const void* pKey, UINT KeySize)
            : data(static_cast<const BYTE*>(pKey), static_cast<const BYTE*>(pKey) + KeySize) {}

        bool operator==(const BlobKey& other) const noexcept
        {
            return data == other.data;
        }
    };

    struct BlobKeyHash
    {
        size_t operator()(const BlobKey& k) const noexcept
        {
            // Keys stored in the database are content hashes with uniform byte
            // distribution, so the first sizeof(size_t) bytes are sufficient for
            // a hash table bucket index.  Correctness is guaranteed by the full
            // equality comparison in BlobKey::operator==.
            size_t h = 0;
            auto n = (std::min)(k.data.size(), sizeof(size_t));
            memcpy(&h, k.data.data(), n);
            return h;
        }
    };

    void Insert(const void* pKey, UINT KeySize, T* value) { std::lock_guard lock(m_lock); m_map[BlobKey(pKey, KeySize)] = value; }

    template<typename TKey>
    void Insert(const TKey* pKey, T* value) { Insert(pKey->pKey, pKey->KeySize, value); }

    HRESULT Find(const void* pKey, UINT KeySize, _COM_Outptr_ T** ppObject)
    {
        std::lock_guard lock(m_lock);
        auto it = m_map.find(BlobKey(pKey, KeySize));
        if (it != m_map.end())
        {
            *ppObject = D3DX12_COM_PTR_GET(it->second);
            (*ppObject)->AddRef();
            return S_OK;
        }
        *ppObject = nullptr;
        return DXGI_ERROR_NOT_FOUND;
    }

    template<typename TKey>
    HRESULT Find(const TKey* pKey, _COM_Outptr_ T** ppObject) { return Find(pKey->pKey, pKey->KeySize, ppObject); }

    void Clear() { std::lock_guard lock(m_lock); m_map.clear(); }

    mutable TLock m_lock;
    std::unordered_map<BlobKey, D3DX12_COM_PTR<T>, BlobKeyHash> m_map;
};

//------------------------------------------------------------------------------------------------
// Owns deep copies of an object key and its associated group keys/versions.
// Use when enumerate callback data must outlive the callback scope (e.g. async dispatch).
// TObjectKey is the typed key struct (e.g. D3D12_DATABASE_PIPELINE_STATE_KEY).
//
template<typename TObjectKey>
struct CD3DX12_DATABASE_KEY_SET
{
    CD3DX12_DATABASE_KEY_SET() = default;

    CD3DX12_DATABASE_KEY_SET(const void* pObjectKey, UINT objectKeySize,
        const D3D12_DATABASE_GROUP_KEY* pGroupKeys, const UINT* pVersions, UINT numGroupKeys)
        : m_objectKeyStorage(static_cast<const BYTE*>(pObjectKey), static_cast<const BYTE*>(pObjectKey) + objectKeySize)
        , m_objectKey{ m_objectKeyStorage.data(), static_cast<UINT>(m_objectKeyStorage.size()) }
        , m_versions(pVersions, pVersions + numGroupKeys)
        , m_groupKeyStorage(numGroupKeys)
        , m_groupKeys(numGroupKeys)
    {
        for (UINT i = 0; i < numGroupKeys; ++i)
        {
            auto pBegin = static_cast<const BYTE*>(pGroupKeys[i].pKey);
            m_groupKeyStorage[i].assign(pBegin, pBegin + pGroupKeys[i].KeySize);
            m_groupKeys[i] = { m_groupKeyStorage[i].data(), pGroupKeys[i].KeySize };
        }
    }

    CD3DX12_DATABASE_KEY_SET(CD3DX12_DATABASE_KEY_SET&&) = default;
    CD3DX12_DATABASE_KEY_SET& operator=(CD3DX12_DATABASE_KEY_SET&&) = default;

    const TObjectKey* ObjectKey() const { return &m_objectKey; }
    const D3D12_DATABASE_GROUP_KEY* GroupKeys() const { return m_groupKeys.data(); }
    const UINT* Versions() const { return m_versions.data(); }
    UINT GroupCount() const { return static_cast<UINT>(m_groupKeys.size()); }

    CD3DX12_DATABASE_KEY_SET(const CD3DX12_DATABASE_KEY_SET&) = delete;
    CD3DX12_DATABASE_KEY_SET& operator=(const CD3DX12_DATABASE_KEY_SET&) = delete;

private:
    std::vector<BYTE> m_objectKeyStorage;
    TObjectKey m_objectKey;
    std::vector<UINT> m_versions;
    std::vector<std::vector<BYTE>> m_groupKeyStorage;
    std::vector<D3D12_DATABASE_GROUP_KEY> m_groupKeys;
};

using CD3DX12_PIPELINE_STATE_KEY_SET = CD3DX12_DATABASE_KEY_SET<D3D12_DATABASE_PIPELINE_STATE_KEY>;
using CD3DX12_STATE_OBJECT_KEY_SET = CD3DX12_DATABASE_KEY_SET<D3D12_DATABASE_STATE_OBJECT_KEY>;

//------------------------------------------------------------------------------------------------
// Helper for enumerating state objects and pipeline states from an ID3D12StateObjectDatabase1.
// Manages root signature and state object caches, and provides static callback functions
// compatible with the enumerate methods.  For state objects, the caller creates the object in
// the visit callback and returns it via ppCreatedObject so the helper can cache it for
// subsequent existing collection and parent lookups.
//
// Usage:
//   CD3DX12_STATE_OBJECT_DATABASE_ENUMERATE_CONTEXT ctx(pDevice, pDatabase1);
//   ctx.EnumeratePipelineStateDescs(MyPipelineStateVisitor, pUserContext);
//   ctx.EnumerateStateObjectDescs(MySOVisitor, pUserContext);
//
template<
    typename TRootSignatureMap = CD3DX12_KEY_TO_OBJECT_MAP<ID3D12RootSignature, CD3DX12_SINGLE_THREADED_LOCK>,
    typename TStateObjectMap = CD3DX12_KEY_TO_OBJECT_MAP<IUnknown, CD3DX12_SINGLE_THREADED_LOCK>>
struct CD3DX12_STATE_OBJECT_DATABASE_ENUMERATE_CONTEXT
{

    CD3DX12_STATE_OBJECT_DATABASE_ENUMERATE_CONTEXT(
        _In_opt_ ID3D12Device* pDevice,
        _In_ ID3D12StateObjectDatabase1* pDatabase,
        BOOL EnableStateObjectLookup = TRUE) noexcept
        : m_pDevice(pDevice)
        , m_pDatabase(pDatabase)
        , m_EnableStateObjectLookup(EnableStateObjectLookup)
    {
    }

    CD3DX12_STATE_OBJECT_DATABASE_ENUMERATE_CONTEXT(const CD3DX12_STATE_OBJECT_DATABASE_ENUMERATE_CONTEXT&) = delete;
    CD3DX12_STATE_OBJECT_DATABASE_ENUMERATE_CONTEXT& operator=(const CD3DX12_STATE_OBJECT_DATABASE_ENUMERATE_CONTEXT&) = delete;

    typedef BOOL(__stdcall* D3DX12PipelineStateFunc)(
        _In_reads_(NumGroupKeys) const D3D12_DATABASE_GROUP_KEY* pGroupKeys,
        _In_reads_(NumGroupKeys) const UINT* pVersions,
        UINT NumGroupKeys,
        _In_ const D3D12_DATABASE_PIPELINE_STATE_KEY* pKey,
        _In_ CD3DX12_STATE_OBJECT_DATABASE_ENUMERATE_CONTEXT* pContext,
        _Inout_opt_ void* pUserContext);

    HRESULT EnumeratePipelineStates(
        _In_ D3DX12PipelineStateFunc pfnCallback,
        _Inout_opt_ void* pUserContext)
    {
        if (!pfnCallback)
        {
            return E_INVALIDARG;
        }
        struct Ctx
        {
            CD3DX12_STATE_OBJECT_DATABASE_ENUMERATE_CONTEXT* pThis;
            D3DX12PipelineStateFunc pfnCallback;
            void* pUserContext;
        } ctx = { this, pfnCallback, pUserContext };

        return GetDatabase()->EnumeratePipelineStates(
            [](const D3D12_DATABASE_GROUP_KEY* pGroupKeys, const UINT* pVersions, UINT NumGroupKeys,
               const D3D12_DATABASE_PIPELINE_STATE_KEY* pKey, void* pCtx) -> BOOL
            {
                auto* p = static_cast<Ctx*>(pCtx);
                return p->pfnCallback(pGroupKeys, pVersions, NumGroupKeys, pKey, p->pThis, p->pUserContext);
            }, &ctx);
    }

    typedef BOOL(__stdcall* D3DX12StateObjectFunc)(
        _In_reads_(NumGroupKeys) const D3D12_DATABASE_GROUP_KEY* pGroupKeys,
        _In_reads_(NumGroupKeys) const UINT* pVersions,
        UINT NumGroupKeys,
        _In_ const D3D12_DATABASE_STATE_OBJECT_KEY* pKey,
        _In_ CD3DX12_STATE_OBJECT_DATABASE_ENUMERATE_CONTEXT* pContext,
        _Inout_opt_ void* pUserContext);

    HRESULT EnumerateStateObjects(
        _In_ D3DX12StateObjectFunc pfnCallback,
        _Inout_opt_ void* pUserContext)
    {
        if (!pfnCallback)
        {
            return E_INVALIDARG;
        }
        struct Ctx
        {
            CD3DX12_STATE_OBJECT_DATABASE_ENUMERATE_CONTEXT* pThis;
            D3DX12StateObjectFunc pfnCallback;
            void* pUserContext;
        } ctx = { this, pfnCallback, pUserContext };

        return GetDatabase()->EnumerateStateObjects(
            [](const D3D12_DATABASE_GROUP_KEY* pGroupKeys, const UINT* pVersions, UINT NumGroupKeys,
               const D3D12_DATABASE_STATE_OBJECT_KEY* pKey, void* pCtx) -> BOOL
            {
                auto* p = static_cast<Ctx*>(pCtx);
                return p->pfnCallback(pGroupKeys, pVersions, NumGroupKeys, pKey, p->pThis, p->pUserContext);
            }, &ctx);
    }

    typedef BOOL(__stdcall* D3DX12PipelineStateDescFunc) (
        _In_reads_(NumGroupKeys) const D3D12_DATABASE_GROUP_KEY* pGroupKeys,
        _In_reads_(NumGroupKeys) const UINT* pVersions,
        UINT NumGroupKeys,
        _In_ const D3D12_DATABASE_PIPELINE_STATE_KEY* pPipelineStateKey,
        _In_ const D3D12_PIPELINE_STATE_STREAM_DESC* pDesc,
        _Inout_opt_ void* pUserContext
        );

    HRESULT EnumeratePipelineStateDescs(
        _In_ D3DX12PipelineStateDescFunc pfnCallback,
        _Inout_opt_ void* pUserContext)
    {
        if (!pfnCallback)
        {
            return E_INVALIDARG;
        }
        EnumeratePipelineStateContext ctx = { this, pfnCallback, pUserContext, S_OK };
        HRESULT hr = m_pDatabase->EnumeratePipelineStates(EnumeratePipelineStateFunc, &ctx);
        return FAILED(ctx.hrResult) ? ctx.hrResult : hr;
    }

    typedef BOOL(__stdcall* D3DX12StateObjectDescFunc) (
        _In_reads_(NumGroupKeys) const D3D12_DATABASE_GROUP_KEY* pGroupKeys,
        _In_reads_(NumGroupKeys) const UINT* pVersions,
        UINT NumGroupKeys,
        _In_ const D3D12_DATABASE_STATE_OBJECT_KEY* pStateObjectKey,
        _In_ const D3D12_STATE_OBJECT_DESC* pDesc,
        _In_opt_ const D3D12_DATABASE_STATE_OBJECT_KEY* pParentKey,
        _In_opt_ const D3D12_DATABASE_GROUP_KEY* pParentGroupKey,
        _COM_Outptr_opt_ IUnknown** ppCreatedObject,
        _Inout_opt_ void* pUserContext
        );

    HRESULT EnumerateStateObjectDescs(
        _In_ D3DX12StateObjectDescFunc pfnCallback,
        _Inout_opt_ void* pUserContext)
    {
        if (!pfnCallback)
        {
            return E_INVALIDARG;
        }
        EnumerateStateObjectContext ctx = { this, pfnCallback, pUserContext, S_OK };
        HRESULT hr = m_pDatabase->EnumerateStateObjects(EnumerateStateObjectFunc, &ctx);
        return FAILED(ctx.hrResult) ? ctx.hrResult : hr;
    }

    HRESULT FindPipelineStateDesc1(
        _In_opt_ const D3D12_DATABASE_GROUP_KEY* pGroupKey,
        _In_opt_ const D3D12_DATABASE_PIPELINE_STATE_KEY* pPipelineStateKey,
        _In_ D3DX12PipelineStateDescFunc pfnCallback,
        _Inout_opt_ void* pUserContext,
        D3D12_FIND_PIPELINE_STATE_DESC_FLAGS Flags = D3D12_FIND_PIPELINE_STATE_DESC_FLAG_NONE,
        _In_reads_opt_(NumEnumGroupKeys) const D3D12_DATABASE_GROUP_KEY* pEnumGroupKeys = nullptr,
        _In_reads_opt_(NumEnumGroupKeys) const UINT* pEnumVersions = nullptr,
        UINT NumEnumGroupKeys = 0)
    {
        if (!pfnCallback)
        {
            return E_INVALIDARG;
        }
        FindPipelineStateContext ctx = { this, pfnCallback, pUserContext, pEnumGroupKeys, pEnumVersions, NumEnumGroupKeys, TRUE };
        return m_pDatabase->FindPipelineStateDesc1(
            pGroupKey, pPipelineStateKey,
            Flags,
            FindPipelineStateFunc,
            m_pDevice ? CreateRootSignature : nullptr,
            m_pDevice ? LookupRootSignature : nullptr,
            &ctx);
    }

    HRESULT FindStateObjectDesc1(
        _In_opt_ const D3D12_DATABASE_GROUP_KEY* pGroupKey,
        _In_opt_ const D3D12_DATABASE_STATE_OBJECT_KEY* pStateObjectKey,
        _In_ D3DX12StateObjectDescFunc pfnCallback,
        _Inout_opt_ void* pUserContext,
        D3D12_FIND_STATE_OBJECT_DESC_FLAGS Flags = D3D12_FIND_STATE_OBJECT_DESC_FLAG_NONE,
        _In_reads_opt_(NumEnumGroupKeys) const D3D12_DATABASE_GROUP_KEY* pEnumGroupKeys = nullptr,
        _In_reads_opt_(NumEnumGroupKeys) const UINT* pEnumVersions = nullptr,
        UINT NumEnumGroupKeys = 0)
    {
        if (!pfnCallback)
        {
            return E_INVALIDARG;
        }
        FindStateObjectContext ctx = { this, pfnCallback, pUserContext, pEnumGroupKeys, pEnumVersions, NumEnumGroupKeys, TRUE };
        return m_pDatabase->FindStateObjectDesc1(
            pGroupKey, pStateObjectKey,
            Flags,
            FindStateObjectFunc,
            m_EnableStateObjectLookup ? LookupStateObject : nullptr,
            m_pDevice ? CreateRootSignature : nullptr,
            m_pDevice ? LookupRootSignature : nullptr,
            &ctx);
    }

    HRESULT FindStateObject(
        _In_ const D3D12_DATABASE_STATE_OBJECT_KEY* pStateObjectKey,
        _In_ REFIID riid,
        _COM_Outptr_ void** ppObject)
    {
        D3DX12_COM_PTR<IUnknown> spFound;
        HRESULT hr = m_stateObjects.Find(pStateObjectKey, D3DX12_COM_PTR_ADDRESSOF(spFound));
        if (FAILED(hr))
        {
            *ppObject = nullptr;
            return hr;
        }
        return spFound->QueryInterface(riid, ppObject);
    }

    TRootSignatureMap m_rootSignatures;
    TStateObjectMap m_stateObjects;

    ID3D12Device* GetDevice() const noexcept { return D3DX12_COM_PTR_GET(m_pDevice); }
    ID3D12StateObjectDatabase1* GetDatabase() const noexcept { return D3DX12_COM_PTR_GET(m_pDatabase); }

private:
    D3DX12_COM_PTR<ID3D12Device> m_pDevice;
    D3DX12_COM_PTR<ID3D12StateObjectDatabase1> m_pDatabase;
    BOOL m_EnableStateObjectLookup;

    // Context structs for Find callbacks (receive desc)
    struct FindPipelineStateContext
    {
        CD3DX12_STATE_OBJECT_DATABASE_ENUMERATE_CONTEXT* pThis;
        D3DX12PipelineStateDescFunc pfnCallback;
        void* pUserContext;
        const D3D12_DATABASE_GROUP_KEY* pEnumGroupKeys;
        const UINT* pEnumVersions;
        UINT NumEnumGroupKeys;
        BOOL bContinue;
    };

    struct FindStateObjectContext
    {
        CD3DX12_STATE_OBJECT_DATABASE_ENUMERATE_CONTEXT* pThis;
        D3DX12StateObjectDescFunc pfnCallback;
        void* pUserContext;
        const D3D12_DATABASE_GROUP_KEY* pEnumGroupKeys;
        const UINT* pEnumVersions;
        UINT NumEnumGroupKeys;
        BOOL bContinue;
    };

    // Context structs for Enumerate callbacks (receive key only)
    struct EnumeratePipelineStateContext
    {
        CD3DX12_STATE_OBJECT_DATABASE_ENUMERATE_CONTEXT* pThis;
        D3DX12PipelineStateDescFunc pfnCallback;
        void* pUserContext;
        HRESULT hrResult;
    };

    struct EnumerateStateObjectContext
    {
        CD3DX12_STATE_OBJECT_DATABASE_ENUMERATE_CONTEXT* pThis;
        D3DX12StateObjectDescFunc pfnCallback;
        void* pUserContext;
        HRESULT hrResult;
    };

    static HRESULT __stdcall LookupRootSignature(
        _In_ const D3D12_DATABASE_ROOT_SIGNATURE_KEY* pRootSignatureKey,
        _COM_Outptr_ ID3D12RootSignature** ppRootSignature,
        _Inout_opt_ void* pContext)
    {
        auto* pThis = static_cast<FindPipelineStateContext*>(pContext)->pThis;
        return pThis->m_rootSignatures.Find(pRootSignatureKey, ppRootSignature);
    }

    static HRESULT __stdcall CreateRootSignature(
        _In_ const D3D12_DATABASE_ROOT_SIGNATURE_KEY* pRootSignatureKey,
        _In_reads_bytes_(SerializedRootSignatureSize) const void* pSerializedRootSignature,
        SIZE_T SerializedRootSignatureSize,
        _COM_Outptr_ ID3D12RootSignature** ppRootSignature,
        _Inout_opt_ void* pContext)
    {
        auto* pThis = static_cast<FindPipelineStateContext*>(pContext)->pThis;
        HRESULT hr = pThis->m_pDevice->CreateRootSignature(
            0, pSerializedRootSignature, SerializedRootSignatureSize,
            IID_ID3D12RootSignature, reinterpret_cast<void**>(ppRootSignature));
        if (SUCCEEDED(hr))
        {
            pThis->m_rootSignatures.Insert(pRootSignatureKey, *ppRootSignature);
        }
        return hr;
    }

    static HRESULT __stdcall LookupStateObject(
        _In_ const D3D12_DATABASE_STATE_OBJECT_KEY* pStateObjectKey,
        _In_ const D3D12_DATABASE_GROUP_KEY* /*pGroupKey*/,
        _COM_Outptr_ IUnknown** ppStateObject,
        _Inout_opt_ void* pContext)
    {
        auto* pThis = static_cast<FindStateObjectContext*>(pContext)->pThis;
        return pThis->m_stateObjects.Find(pStateObjectKey, ppStateObject);
    }

    // Find callbacks (receive desc, forward to user visitor with enumerate-provided groups)
    static void __stdcall FindPipelineStateFunc(
        _In_reads_(NumGroupKeys) const D3D12_DATABASE_GROUP_KEY* pGroupKeys,
        _In_reads_(NumGroupKeys) const UINT* pVersions,
        UINT NumGroupKeys,
        _In_ const D3D12_DATABASE_PIPELINE_STATE_KEY* pPipelineStateKey,
        _In_ const D3D12_PIPELINE_STATE_STREAM_DESC* pDesc,
        _Inout_opt_ void* pContext)
    {
        auto* pCtx = static_cast<FindPipelineStateContext*>(pContext);
        // Use enumerate-provided groups if available, otherwise use Find-provided groups
        const D3D12_DATABASE_GROUP_KEY* pGroups = pCtx->NumEnumGroupKeys > 0 ? pCtx->pEnumGroupKeys : pGroupKeys;
        const UINT* pVers = pCtx->NumEnumGroupKeys > 0 ? pCtx->pEnumVersions : pVersions;
        UINT numGroups = pCtx->NumEnumGroupKeys > 0 ? pCtx->NumEnumGroupKeys : NumGroupKeys;
        pCtx->bContinue = pCtx->pfnCallback(
            pGroups, pVers, numGroups,
            pPipelineStateKey,
            pDesc, pCtx->pUserContext);
    }

    static void __stdcall FindStateObjectFunc(
        _In_reads_(NumGroupKeys) const D3D12_DATABASE_GROUP_KEY* pGroupKeys,
        _In_reads_(NumGroupKeys) const UINT* pVersions,
        UINT NumGroupKeys,
        _In_ const D3D12_DATABASE_STATE_OBJECT_KEY* pStateObjectKey,
        _In_ const D3D12_STATE_OBJECT_DESC* pDesc,
        _In_opt_ const D3D12_DATABASE_STATE_OBJECT_KEY* pParentKey,
        _In_opt_ const D3D12_DATABASE_GROUP_KEY* pParentGroupKey,
        _Inout_opt_ void* pContext)
    {
        auto* pCtx = static_cast<FindStateObjectContext*>(pContext);
        auto* pThis = pCtx->pThis;
        const D3D12_DATABASE_GROUP_KEY* pGroups = pCtx->NumEnumGroupKeys > 0 ? pCtx->pEnumGroupKeys : pGroupKeys;
        const UINT* pVers = pCtx->NumEnumGroupKeys > 0 ? pCtx->pEnumVersions : pVersions;
        UINT numGroups = pCtx->NumEnumGroupKeys > 0 ? pCtx->NumEnumGroupKeys : NumGroupKeys;
        IUnknown* pCreated = nullptr;
        pCtx->bContinue = pCtx->pfnCallback(
            pGroups, pVers, numGroups,
            pStateObjectKey,
            pDesc, pParentKey, pParentGroupKey,
            &pCreated, pCtx->pUserContext);
        if (pCreated)
        {
            pThis->m_stateObjects.Insert(pStateObjectKey, pCreated);
            pCreated->Release();
        }
    }

    // Enumerate callbacks (receive key + groups, call Find per key with NO_GROUPS)
    static BOOL __stdcall EnumeratePipelineStateFunc(
        _In_reads_(NumGroupKeys) const D3D12_DATABASE_GROUP_KEY* pGroupKeys,
        _In_reads_(NumGroupKeys) const UINT* pVersions,
        UINT NumGroupKeys,
        _In_ const D3D12_DATABASE_PIPELINE_STATE_KEY* pPipelineStateKey,
        _Inout_opt_ void* pContext)
    {
        auto* pCtx = static_cast<EnumeratePipelineStateContext*>(pContext);
        FindPipelineStateContext findCtx = { pCtx->pThis, pCtx->pfnCallback, pCtx->pUserContext,
                                   pGroupKeys, pVersions, NumGroupKeys, TRUE };
        pCtx->hrResult = pCtx->pThis->m_pDatabase->FindPipelineStateDesc1(
            nullptr, pPipelineStateKey,
            D3D12_FIND_PIPELINE_STATE_DESC_FLAG_NO_GROUPS,
            FindPipelineStateFunc,
            pCtx->pThis->m_pDevice ? CreateRootSignature : nullptr,
            pCtx->pThis->m_pDevice ? LookupRootSignature : nullptr,
            &findCtx);
        return SUCCEEDED(pCtx->hrResult) && findCtx.bContinue;
    }

    static BOOL __stdcall EnumerateStateObjectFunc(
        _In_reads_(NumGroupKeys) const D3D12_DATABASE_GROUP_KEY* pGroupKeys,
        _In_reads_(NumGroupKeys) const UINT* pVersions,
        UINT NumGroupKeys,
        _In_ const D3D12_DATABASE_STATE_OBJECT_KEY* pStateObjectKey,
        _Inout_opt_ void* pContext)
    {
        auto* pCtx = static_cast<EnumerateStateObjectContext*>(pContext);
        FindStateObjectContext findCtx = { pCtx->pThis, pCtx->pfnCallback, pCtx->pUserContext,
                                  pGroupKeys, pVersions, NumGroupKeys, TRUE };
        pCtx->hrResult = pCtx->pThis->m_pDatabase->FindStateObjectDesc1(
            nullptr, pStateObjectKey,
            D3D12_FIND_STATE_OBJECT_DESC_FLAG_NO_GROUPS,
            FindStateObjectFunc,
            pCtx->pThis->m_EnableStateObjectLookup ? LookupStateObject : nullptr,
            pCtx->pThis->m_pDevice ? CreateRootSignature : nullptr,
            pCtx->pThis->m_pDevice ? LookupRootSignature : nullptr,
            &findCtx);
        return SUCCEEDED(pCtx->hrResult) && findCtx.bContinue;
    }
};

//------------------------------------------------------------------------------------------------
// RAII helper for managing a transaction on an ID3D12StateObjectDatabase1.
// Automatically rolls back on destruction if the transaction was not committed
// or rolled back.  Pass D3DX12_DATABASE_TRANSACTION_FLAGS::COMMIT_ON_CLOSE to auto-commit instead.
//
// Usage:
//   CD3DX12_DATABASE_TRANSACTION transaction(pDatabase);
//   transaction.Begin(D3D12_TRANSACTION_TYPE_IMMEDIATE);
//   pDatabase->StorePipelineStateDesc(...);
//   transaction.Commit();
//
enum class D3DX12_DATABASE_TRANSACTION_FLAGS
{
    NONE = 0,
    COMMIT_ON_CLOSE = 0x1,
};
DEFINE_ENUM_FLAG_OPERATORS(D3DX12_DATABASE_TRANSACTION_FLAGS)

struct CD3DX12_DATABASE_TRANSACTION
{
    CD3DX12_DATABASE_TRANSACTION(
        _In_ ID3D12StateObjectDatabase1* pDatabase,
        D3DX12_DATABASE_TRANSACTION_FLAGS flags = D3DX12_DATABASE_TRANSACTION_FLAGS::NONE) noexcept
        : m_pDatabase(pDatabase)
        , m_State(State::None)
        , m_Flags(flags)
    {
    }

    ~CD3DX12_DATABASE_TRANSACTION()
    {
        if (m_State == State::Open)
        {
            if ((m_Flags & D3DX12_DATABASE_TRANSACTION_FLAGS::COMMIT_ON_CLOSE) != D3DX12_DATABASE_TRANSACTION_FLAGS::NONE)
            {
                m_pDatabase->CommitTransaction();
            }
            else
            {
                m_pDatabase->RollbackTransaction();
            }
        }
    }

    HRESULT Begin(D3D12_TRANSACTION_TYPE Type = D3D12_TRANSACTION_TYPE_DEFERRED) noexcept
    {
        if (m_State != State::None)
        {
            return E_ILLEGAL_STATE_CHANGE;
        }
        HRESULT hr = m_pDatabase->BeginTransaction(Type);
        if (SUCCEEDED(hr))
        {
            m_State = State::Open;
        }
        return hr;
    }

    HRESULT Commit() noexcept
    {
        if (m_State != State::Open)
        {
            return E_ILLEGAL_STATE_CHANGE;
        }
        HRESULT hr = m_pDatabase->CommitTransaction();
        if (SUCCEEDED(hr))
        {
            m_State = State::Closed;
        }
        return hr;
    }

    HRESULT Rollback() noexcept
    {
        if (m_State != State::Open)
        {
            return E_ILLEGAL_STATE_CHANGE;
        }
        HRESULT hr = m_pDatabase->RollbackTransaction();
        if (SUCCEEDED(hr))
        {
            m_State = State::Closed;
        }
        return hr;
    }

    CD3DX12_DATABASE_TRANSACTION(const CD3DX12_DATABASE_TRANSACTION&) = delete;
    CD3DX12_DATABASE_TRANSACTION& operator=(const CD3DX12_DATABASE_TRANSACTION&) = delete;

private:
    enum class State { None, Open, Closed };

    D3DX12_COM_PTR<ID3D12StateObjectDatabase1> m_pDatabase;
    State m_State;
    D3DX12_DATABASE_TRANSACTION_FLAGS m_Flags;
};

#endif // D3D12_SDK_VERSION >= 620
