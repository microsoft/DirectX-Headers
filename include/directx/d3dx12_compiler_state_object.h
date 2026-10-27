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

#include "d3d12compiler.h"
#include "d3dx12_state_object.h"

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

//================================================================================================
// D3DX12 compiler State Object Creation Helpers
//================================================================================================
#if defined(D3D12_SDK_VERSION) && (D3D12_SDK_VERSION >= 616)
//------------------------------------------------------------------------------------------------
class CD3DX12_COMPILER_EXISTING_COLLECTION_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_COMPILER_EXISTING_COLLECTION_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_COMPILER_EXISTING_COLLECTION_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_COMPILER_EXISTING_COLLECTION_SUBOBJECT(const CD3DX12_COMPILER_EXISTING_COLLECTION_SUBOBJECT& other) = delete;
    CD3DX12_COMPILER_EXISTING_COLLECTION_SUBOBJECT& operator=(const CD3DX12_COMPILER_EXISTING_COLLECTION_SUBOBJECT& other) = delete;
    CD3DX12_COMPILER_EXISTING_COLLECTION_SUBOBJECT(CD3DX12_COMPILER_EXISTING_COLLECTION_SUBOBJECT&& other) = default;
    CD3DX12_COMPILER_EXISTING_COLLECTION_SUBOBJECT& operator=(CD3DX12_COMPILER_EXISTING_COLLECTION_SUBOBJECT&& other) = default;
    void SetExistingCollection(ID3D12CompilerStateObject* pExistingCollection) noexcept
    {
        m_Desc.pExistingCollection = pExistingCollection;
        m_CollectionRef = pExistingCollection;
    }
    void DefineExport(
        LPCWSTR Name,
        LPCWSTR ExportToRename = nullptr,
        D3D12_EXPORT_FLAGS Flags = D3D12_EXPORT_FLAG_NONE)
    {
        D3D12_EXPORT_DESC Export;
        Export.Name = m_Strings.LocalCopy(Name);
        Export.ExportToRename = m_Strings.LocalCopy(ExportToRename);
        Export.Flags = Flags;
        m_Exports.push_back(Export);
        m_Desc.pExports = &m_Exports[0]; // using ugly way to get pointer in case .data() is not defined
        m_Desc.NumExports = static_cast<UINT>(m_Exports.size());
    }
    template<size_t N>
    void DefineExports(LPCWSTR(&Exports)[N])
    {
        for (UINT i = 0; i < N; i++)
        {
            DefineExport(Exports[i]);
        }
    }
    void DefineExports(const LPCWSTR* Exports, UINT N)
    {
        for (UINT i = 0; i < N; i++)
        {
            DefineExport(Exports[i]);
        }
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION;
    }
    operator const D3D12_COMPILER_EXISTING_COLLECTION_DESC&() const noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        m_CollectionRef = nullptr;
        m_Strings.clear();
        m_Exports.clear();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_COMPILER_EXISTING_COLLECTION_DESC m_Desc;
    D3DX12_COM_PTR<ID3D12CompilerStateObject> m_CollectionRef;
    CD3DX12_STATE_OBJECT_DESC::StringContainer<LPCWSTR, std::wstring> m_Strings;
    std::vector<D3D12_EXPORT_DESC> m_Exports;
};
#endif // defined(D3D12_SDK_VERSION) && (D3D12_SDK_VERSION >= 616)
