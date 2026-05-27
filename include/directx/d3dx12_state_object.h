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

#include "d3dx12_default.h"
#include "d3d12.h"
#include "d3dx12_core.h"

#include "d3d12shader.h"

//================================================================================================
// D3DX12 State Object Creation Helpers
//
// Helper classes for creating new style state objects out of an arbitrary set of subobjects.
// Uses STL
//
// Start by instantiating CD3DX12_STATE_OBJECT_DESC (see its public methods).
// One of its methods is CreateSubobject(), which has a comment showing a couple of options for
// defining subobjects using the helper classes for each subobject (CD3DX12_DXIL_LIBRARY_SUBOBJECT
// etc.). The subobject helpers each have methods specific to the subobject for configuring its
// contents.
//
//================================================================================================
#include <list>
#include <forward_list>
#include <vector>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
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

//------------------------------------------------------------------------------------------------
class CD3DX12_STATE_OBJECT_DESC
{
public:
    CD3DX12_STATE_OBJECT_DESC() noexcept
    {
        Init(D3D12_STATE_OBJECT_TYPE_COLLECTION);
    }
    CD3DX12_STATE_OBJECT_DESC(D3D12_STATE_OBJECT_TYPE Type) noexcept
    {
        Init(Type);
    }
    void SetStateObjectType(D3D12_STATE_OBJECT_TYPE Type) noexcept { m_Desc.Type = Type; }

    // Deep-copy an existing D3D12_STATE_OBJECT_DESC into this builder.
    // Bytecode pointers (DXIL libraries) are copied by pointer, not deep-copied.
    HRESULT InitFromDesc(const D3D12_STATE_OBJECT_DESC& desc);


    CD3DX12_STATE_OBJECT_DESC(const CD3DX12_STATE_OBJECT_DESC& other) = delete;
    CD3DX12_STATE_OBJECT_DESC& operator=(const CD3DX12_STATE_OBJECT_DESC& other) = delete;
    CD3DX12_STATE_OBJECT_DESC(CD3DX12_STATE_OBJECT_DESC&& other) = default;
    CD3DX12_STATE_OBJECT_DESC& operator=(CD3DX12_STATE_OBJECT_DESC&& other) = default;
    operator const D3D12_STATE_OBJECT_DESC& ()
    {
#if defined(D3D12_SDK_VERSION) && (D3D12_SDK_VERSION >= 612)
        m_RepointedSubobjectVectors.clear();
        m_RepointedPrograms.clear();
#endif
        m_RepointedPartialPrograms.clear();
        m_RepointedAssociations.clear();
        m_SubobjectArray.clear();
        m_SubobjectArray.reserve(m_Desc.NumSubobjects);
        // Flatten subobjects into an array (each flattened subobject still has a
        // member that's a pointer to its desc that's not flattened)
        for (auto Iter = m_SubobjectList.begin();
            Iter != m_SubobjectList.end(); Iter++)
        {
            m_SubobjectArray.push_back(*Iter);
            // Store new location in array so we can redirect pointers contained in subobjects
            Iter->pSubobjectArrayLocation = &m_SubobjectArray.back();
        }
        for (UINT i = 0; i < m_Desc.NumSubobjects; i++)
        {
            if (m_SubobjectArray[i].Type == D3D12_STATE_SUBOBJECT_TYPE_PARTIAL_GRAPHICS_PROGRAM)
            {
                auto originalPartialProgramDesc =
                    static_cast<const D3D12_PARTIAL_GRAPHICS_PROGRAM_DESC*>(m_SubobjectArray[i].pDesc);
                D3D12_PARTIAL_GRAPHICS_PROGRAM_DESC Repointed = *originalPartialProgramDesc;
                if (originalPartialProgramDesc->NumSubobjects > 0)
                {
                    m_RepointedSubobjectVectors.emplace_back(std::vector<const D3D12_STATE_SUBOBJECT*>());
                    std::vector<D3D12_STATE_SUBOBJECT const*>& repointedPartialProgramSubobjects = m_RepointedSubobjectVectors.back();
                    repointedPartialProgramSubobjects.resize(originalPartialProgramDesc->NumSubobjects);
                    for (UINT s = 0; s < originalPartialProgramDesc->NumSubobjects; s++)
                    {
                        auto pWrapper =
                            static_cast<const SUBOBJECT_WRAPPER*>(originalPartialProgramDesc->ppSubobjects[s]);
                        repointedPartialProgramSubobjects[s] = pWrapper->pSubobjectArrayLocation;
                    }
                    // Below: using ugly way to get pointer in case .data() is not defined
                    Repointed.ppSubobjects = &repointedPartialProgramSubobjects[0];
                }
                m_RepointedPartialPrograms.push_back(Repointed);
                m_SubobjectArray[i].pDesc = &m_RepointedPartialPrograms.back();
            }
        }
        // For subobjects with pointer fields, create a new copy of those subobject definitions
        // with fixed pointers
        for (UINT i = 0; i < m_Desc.NumSubobjects; i++)
        {
            if (m_SubobjectArray[i].Type == D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION)
            {
                auto pOriginalSubobjectAssociation =
                    static_cast<const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(m_SubobjectArray[i].pDesc);
                D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION Repointed = *pOriginalSubobjectAssociation;
                auto pWrapper =
                    static_cast<const SUBOBJECT_WRAPPER*>(pOriginalSubobjectAssociation->pSubobjectToAssociate);
                Repointed.pSubobjectToAssociate = pWrapper->pSubobjectArrayLocation;
                m_RepointedAssociations.push_back(Repointed);
                m_SubobjectArray[i].pDesc = &m_RepointedAssociations.back();
            }
#if defined(D3D12_SDK_VERSION) && (D3D12_SDK_VERSION >= 612)
            else if (m_SubobjectArray[i].Type == D3D12_STATE_SUBOBJECT_TYPE_GENERIC_PROGRAM)
            {
                auto originalGenericProgramDesc =
                    static_cast<const D3D12_GENERIC_PROGRAM_DESC*>(m_SubobjectArray[i].pDesc);
                D3D12_GENERIC_PROGRAM_DESC Repointed = *originalGenericProgramDesc;
                if (originalGenericProgramDesc->NumSubobjects > 0)
                {
                    m_RepointedSubobjectVectors.emplace_back(std::vector<const D3D12_STATE_SUBOBJECT*>());
                    std::vector<D3D12_STATE_SUBOBJECT const*>& repointedGenericProgramSubobjects = m_RepointedSubobjectVectors.back();
                    repointedGenericProgramSubobjects.resize(originalGenericProgramDesc->NumSubobjects);
                    for (UINT s = 0; s < originalGenericProgramDesc->NumSubobjects; s++)
                    {
                        auto pWrapper =
                                    static_cast<const SUBOBJECT_WRAPPER*>(originalGenericProgramDesc->ppSubobjects[s]);
                       repointedGenericProgramSubobjects[s] = pWrapper->pSubobjectArrayLocation;
                    }
                    // Below: using ugly way to get pointer in case .data() is not defined
                    Repointed.ppSubobjects = &repointedGenericProgramSubobjects[0];
                }
                m_RepointedPrograms.push_back(Repointed);
                m_SubobjectArray[i].pDesc = &m_RepointedPrograms.back();
            }
#endif
        }

        // Below: using ugly way to get pointer in case .data() is not defined
        m_Desc.pSubobjects = m_Desc.NumSubobjects ? &m_SubobjectArray[0] : nullptr;
        return m_Desc;
    }
    operator const D3D12_STATE_OBJECT_DESC* ()
    {
        // Cast calls the above final preparation work
        return &static_cast<const D3D12_STATE_OBJECT_DESC&>(*this);
    }

    // CreateSubobject creates a sububject helper (e.g. CD3DX12_HIT_GROUP_SUBOBJECT)
    // whose lifetime is owned by this class.
    // e.g.
    //
    //    CD3DX12_STATE_OBJECT_DESC Collection1(D3D12_STATE_OBJECT_TYPE_COLLECTION);
    //    auto Lib0 = Collection1.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    //    Lib0->SetDXILLibrary(&pMyAppDxilLibs[0]);
    //    Lib0->DefineExport(L"rayGenShader0"); // in practice these export listings might be
    //                                          // data/engine driven
    //    etc.
    //
    // Alternatively, users can instantiate sububject helpers explicitly, such as via local
    // variables instead, passing the state object desc that should point to it into the helper
    // constructor (or call mySubobjectHelper.AddToStateObject(Collection1)).
    // In this alternative scenario, the user must keep the subobject alive as long as the state
    // object it is associated with is alive, else its pointer references will be stale.
    // e.g.
    //
    //    CD3DX12_STATE_OBJECT_DESC RaytracingState2(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);
    //    CD3DX12_DXIL_LIBRARY_SUBOBJECT LibA(RaytracingState2);
    //    LibA.SetDXILLibrary(&pMyAppDxilLibs[4]); // not manually specifying exports
    //                                             // - meaning all exports in the libraries
    //                                             // are exported
    //    etc.

    template<typename T>
    T* CreateSubobject()
    {
        T* pSubobject = new T(*this);
        m_OwnedSubobjectHelpers.emplace_back(pSubobject);
        return pSubobject;
    }

    template<typename T, typename U>
    T* CreateSubobject(U&& arg)
    {
        T* pSubobject = new T(std::forward<U>(arg), *this);
        m_OwnedSubobjectHelpers.emplace_back(pSubobject);
        return pSubobject;
    }

    // Iterate over all subobjects of a given type, calling a callback with a reference
    // to the concrete subobject helper.
    template<typename T, typename Fn>
    void ForEachSubobject(D3D12_STATE_SUBOBJECT_TYPE type, Fn&& callback)
    {
        for (auto& helper : m_OwnedSubobjectHelpers)
        {
            if (helper->Type() == type)
            {
                callback(static_cast<T&>(*helper));
            }
        }
    }

private:
    D3D12_STATE_SUBOBJECT* TrackSubobject(D3D12_STATE_SUBOBJECT_TYPE Type, void* pDesc)
    {
        SUBOBJECT_WRAPPER Subobject;
        Subobject.pSubobjectArrayLocation = nullptr;
        Subobject.Type = Type;
        Subobject.pDesc = pDesc;
        m_SubobjectList.push_back(Subobject);
        m_Desc.NumSubobjects++;
        return &m_SubobjectList.back();
    }
    void Init(D3D12_STATE_OBJECT_TYPE Type) noexcept
    {
        SetStateObjectType(Type);
        m_Desc.pSubobjects = nullptr;
        m_Desc.NumSubobjects = 0;
        m_SubobjectList.clear();
        m_SubobjectArray.clear();
        m_RepointedAssociations.clear();
#if defined(D3D12_SDK_VERSION) && (D3D12_SDK_VERSION >= 612)
        m_RepointedSubobjectVectors.clear();
        m_RepointedPrograms.clear();
#endif
        m_RepointedPartialPrograms.clear();
    }
    typedef struct SUBOBJECT_WRAPPER : public D3D12_STATE_SUBOBJECT
    {
        D3D12_STATE_SUBOBJECT* pSubobjectArrayLocation; // new location when flattened into array
                                                        // for repointing pointers in subobjects
    } SUBOBJECT_WRAPPER;
    D3D12_STATE_OBJECT_DESC m_Desc;
    std::list<SUBOBJECT_WRAPPER>   m_SubobjectList; // Pointers to list nodes handed out so
                                                    // these can be edited live
    std::vector<D3D12_STATE_SUBOBJECT> m_SubobjectArray; // Built at the end, copying list contents

    std::list<D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION>
        m_RepointedAssociations; // subobject type that contains pointers to other subobjects,
                                 // repointed to flattened array

#if defined(D3D12_SDK_VERSION) && (D3D12_SDK_VERSION >= 612)
    std::list<std::vector<D3D12_STATE_SUBOBJECT const*>>
        m_RepointedSubobjectVectors;
    std::list<D3D12_GENERIC_PROGRAM_DESC>
        m_RepointedPrograms;
#endif
std::list<D3D12_PARTIAL_GRAPHICS_PROGRAM_DESC>
        m_RepointedPartialPrograms;

    template<typename CStr, typename StdStr>
    class StringContainer
    {
    public:
        CStr LocalCopy(CStr string, bool bSingleString = false)
        {
            if (string)
            {
                if (bSingleString)
                {
                    m_Strings.clear();
                    m_Strings.push_back(string);
                }
                else
                {
                    m_Strings.push_back(string);
                }
                return m_Strings.back().c_str();
            }
            else
            {
                return nullptr;
            }
        }
        void clear() noexcept { m_Strings.clear(); }
    private:
        std::list<StdStr> m_Strings;
    };

public:
    class SUBOBJECT_HELPER_BASE
    {
    public:
        SUBOBJECT_HELPER_BASE() noexcept { Init(); }
        virtual ~SUBOBJECT_HELPER_BASE() = default;
        virtual D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept = 0;
        SUBOBJECT_HELPER_BASE(const SUBOBJECT_HELPER_BASE& other) = delete;
        SUBOBJECT_HELPER_BASE& operator=(const SUBOBJECT_HELPER_BASE& other) = delete;
        SUBOBJECT_HELPER_BASE(SUBOBJECT_HELPER_BASE&& other) = default;
        SUBOBJECT_HELPER_BASE& operator=(SUBOBJECT_HELPER_BASE&& other) = default;
        void AddToStateObject(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        {
            m_pSubobject = ContainingStateObject.TrackSubobject(Type(), Data());
        }
        operator const D3D12_STATE_SUBOBJECT& () const noexcept { return *m_pSubobject; }
    protected:
        virtual void* Data() noexcept = 0;
        void Init() noexcept { m_pSubobject = nullptr; }
        D3D12_STATE_SUBOBJECT* m_pSubobject;
    };

private:
    std::list<std::unique_ptr<SUBOBJECT_HELPER_BASE>> m_OwnedSubobjectHelpers;

    friend class CD3DX12_DXIL_LIBRARY_SUBOBJECT;
    friend class CD3DX12_EXISTING_COLLECTION_SUBOBJECT;
    friend class CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT;
    friend class CD3DX12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
    friend class CD3DX12_HIT_GROUP_SUBOBJECT;
    friend class CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT;
    friend class CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT;
    friend class CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT;
    friend class CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT;
    friend class CD3DX12_API_EXTENSION_SUBOBJECT;
    friend class CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT;
    friend class CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT;
    friend class CD3DX12_NODE_MASK_SUBOBJECT;
    //TODO: SDK Version check should include all the newly added subobject type for the public release.
    // The SDK version check will be changed based on when we release state objects.
#if defined(D3D12_SDK_VERSION) && (D3D12_SDK_VERSION >= 612)
    friend class CD3DX12_GENERIC_PROGRAM_SUBOBJECT;
    friend class CD3DX12_WORK_GRAPH_SUBOBJECT;
    friend class CD3DX12_STREAM_OUTPUT_SUBOBJECT;
    friend class CD3DX12_BLEND_SUBOBJECT;
    friend class CD3DX12_RASTERIZER_SUBOBJECT;
    friend class CD3DX12_DEPTH_STENCIL2_SUBOBJECT;
    friend class CD3DX12_INPUT_LAYOUT_SUBOBJECT;
    friend class CD3DX12_IB_STRIP_CUT_VALUE_SUBOBJECT;
    friend class CD3DX12_PRIMITIVE_TOPOLOGY_SUBOBJECT;
    friend class CD3DX12_RENDER_TARGET_FORMATS_SUBOBJECT;
    friend class CD3DX12_DEPTH_STENCIL_FORMAT_SUBOBJECT;
    friend class CD3DX12_SAMPLE_DESC_SUBOBJECT;
    friend class CD3DX12_FLAGS_SUBOBJECT;
    friend class CD3DX12_VIEW_INSTANCING_SUBOBJECT;
    friend class CD3DX12_DEPTH_STENCIL_SUBOBJECT;
    friend class CD3DX12_DEPTH_STENCIL1_SUBOBJECT;
    friend class CD3DX12_SAMPLE_MASK_SUBOBJECT;
    friend class CD3DX12_NODE_OUTPUT_OVERRIDES;
    friend class CD3DX12_NODE_HELPER_BASE;
    friend class CD3DX12_SHADER_NODE;
    friend class CD3DX12_BROADCASTING_LAUNCH_NODE_OVERRIDES;
    friend class CD3DX12_COALESCING_LAUNCH_NODE_OVERRIDES;
    friend class CD3DX12_THREAD_LAUNCH_NODE_OVERRIDES;
    friend class CD3DX12_COMMON_COMPUTE_NODE_OVERRIDES;
#endif // D3D12_SDK_VERSION >= 612
#if defined(D3D12_PREVIEW_SDK_VERSION) && (D3D12_PREVIEW_SDK_VERSION >= 713)
    friend class CD3DX12_PROGRAM_NODE;
    friend class CD3DX12_MESH_LAUNCH_NODE_OVERRIDES;
    friend class CD3DX12_COMMON_PROGRAM_NODE_OVERRIDES;
#endif // D3D12_PREVIEW_SDK_VERSION >= 713
#if defined(D3D12_SDK_VERSION) && (D3D12_SDK_VERSION >= 618)
    friend class CD3DX12_GLOBAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT;
    friend class CD3DX12_LOCAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT;
    friend class CD3DX12_COMPILER_EXISTING_COLLECTION_SUBOBJECT;
    friend class CD3DX12_EXISTING_COLLECTION_BY_KEY_SUBOBJECT;
#endif
    // todo: add if define checks for sdk version
    friend class CD3DX12_PARTIAL_GRAPHICS_PROGRAM_SUBOBJECT;
    friend class CD3DX12_OUTPUT_LINKAGE_SIGNATURE_SUBOBJECT;
    friend class CD3DX12_PRERASTERIZATION_OUTPUT_LINKAGE_SIGNATURE_SUBOBJECT;
    friend class CD3DX12_PRERASTERIZATION_SHADERS_PARTIAL_PROGRAM_FIELDS_SUBOBJECT;
    friend class CD3DX12_PIXEL_SHADER_PARTIAL_PROGRAM_FIELDS_SUBOBJECT;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_DXIL_LIBRARY_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_DXIL_LIBRARY_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_DXIL_LIBRARY_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_DXIL_LIBRARY_SUBOBJECT(const CD3DX12_DXIL_LIBRARY_SUBOBJECT& other) = delete;
    CD3DX12_DXIL_LIBRARY_SUBOBJECT& operator=(const CD3DX12_DXIL_LIBRARY_SUBOBJECT& other) = delete;
    CD3DX12_DXIL_LIBRARY_SUBOBJECT(CD3DX12_DXIL_LIBRARY_SUBOBJECT&& other) = default;
    CD3DX12_DXIL_LIBRARY_SUBOBJECT& operator=(CD3DX12_DXIL_LIBRARY_SUBOBJECT&& other) = default;
    void SetDXILLibrary(const D3D12_SHADER_BYTECODE* pCode) noexcept
    {
        static const D3D12_SHADER_BYTECODE Default = {};
        m_Desc.DXILLibrary = pCode ? *pCode : Default;
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
        m_Desc.pExports = &m_Exports[0];  // using ugly way to get pointer in case .data() is not defined
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
        return D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    }
    operator const D3D12_DXIL_LIBRARY_DESC&() const noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        m_Strings.clear();
        m_Exports.clear();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_DXIL_LIBRARY_DESC m_Desc;
    CD3DX12_STATE_OBJECT_DESC::StringContainer<LPCWSTR, std::wstring> m_Strings;
    std::vector<D3D12_EXPORT_DESC> m_Exports;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_EXISTING_COLLECTION_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_EXISTING_COLLECTION_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_EXISTING_COLLECTION_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_EXISTING_COLLECTION_SUBOBJECT(const CD3DX12_EXISTING_COLLECTION_SUBOBJECT& other) = delete;
    CD3DX12_EXISTING_COLLECTION_SUBOBJECT& operator=(const CD3DX12_EXISTING_COLLECTION_SUBOBJECT& other) = delete;
    CD3DX12_EXISTING_COLLECTION_SUBOBJECT(CD3DX12_EXISTING_COLLECTION_SUBOBJECT&& other) = default;
    CD3DX12_EXISTING_COLLECTION_SUBOBJECT& operator=(CD3DX12_EXISTING_COLLECTION_SUBOBJECT&& other) = default;
    void SetExistingCollection(ID3D12StateObject*pExistingCollection) noexcept
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
    operator const D3D12_EXISTING_COLLECTION_DESC&() const noexcept { return m_Desc; }
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
    D3D12_EXISTING_COLLECTION_DESC m_Desc;
    D3DX12_COM_PTR<ID3D12StateObject> m_CollectionRef;
    CD3DX12_STATE_OBJECT_DESC::StringContainer<LPCWSTR, std::wstring> m_Strings;
    std::vector<D3D12_EXPORT_DESC> m_Exports;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT(const CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT& other) = delete;
    CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT& operator=(const CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT& other) = delete;
    CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT(CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT&& other) = default;
    CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT& operator=(CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT&& other) = default;
    void SetSubobjectToAssociate(const D3D12_STATE_SUBOBJECT& SubobjectToAssociate) noexcept
    {
        m_Desc.pSubobjectToAssociate = &SubobjectToAssociate;
    }
    void AddExport(LPCWSTR Export)
    {
        m_Desc.NumExports++;
        m_Exports.push_back(m_Strings.LocalCopy(Export));
        m_Desc.pExports = &m_Exports[0];  // using ugly way to get pointer in case .data() is not defined
    }
    template<size_t N>
    void AddExports(LPCWSTR (&Exports)[N])
    {
        for (UINT i = 0; i < N; i++)
        {
            AddExport(Exports[i]);
        }
    }
    void AddExports(const LPCWSTR* Exports, UINT N)
    {
        for (UINT i = 0; i < N; i++)
        {
            AddExport(Exports[i]);
        }
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
    }
    operator const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION&() const noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        m_Strings.clear();
        m_Exports.clear();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION m_Desc;
    CD3DX12_STATE_OBJECT_DESC::StringContainer<LPCWSTR, std::wstring> m_Strings;
    std::vector<LPCWSTR> m_Exports;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION() noexcept
    {
        Init();
    }
    CD3DX12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION(const CD3DX12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION& other) = delete;
    CD3DX12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION& operator=(const CD3DX12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION& other) = delete;
    CD3DX12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION(CD3DX12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION&& other) = default;
    CD3DX12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION& operator=(CD3DX12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION&& other) = default;
    void SetSubobjectNameToAssociate(LPCWSTR SubobjectToAssociate)
    {
        m_Desc.SubobjectToAssociate = m_SubobjectName.LocalCopy(SubobjectToAssociate, true);
    }
    void AddExport(LPCWSTR Export)
    {
        m_Desc.NumExports++;
        m_Exports.push_back(m_Strings.LocalCopy(Export));
        m_Desc.pExports = &m_Exports[0];  // using ugly way to get pointer in case .data() is not defined
    }
    template<size_t N>
    void AddExports(LPCWSTR (&Exports)[N])
    {
        for (UINT i = 0; i < N; i++)
        {
            AddExport(Exports[i]);
        }
    }
    void AddExports(const LPCWSTR* Exports, UINT N)
    {
        for (UINT i = 0; i < N; i++)
        {
            AddExport(Exports[i]);
        }
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
    }
    operator const D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION&() const noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        m_Strings.clear();
        m_SubobjectName.clear();
        m_Exports.clear();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION m_Desc;
    CD3DX12_STATE_OBJECT_DESC::StringContainer<LPCWSTR, std::wstring> m_Strings;
    CD3DX12_STATE_OBJECT_DESC::StringContainer<LPCWSTR, std::wstring> m_SubobjectName;
    std::vector<LPCWSTR> m_Exports;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_HIT_GROUP_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_HIT_GROUP_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_HIT_GROUP_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_HIT_GROUP_SUBOBJECT(const CD3DX12_HIT_GROUP_SUBOBJECT& other) = delete;
    CD3DX12_HIT_GROUP_SUBOBJECT& operator=(const CD3DX12_HIT_GROUP_SUBOBJECT& other) = delete;
    CD3DX12_HIT_GROUP_SUBOBJECT(CD3DX12_HIT_GROUP_SUBOBJECT&& other) = default;
    CD3DX12_HIT_GROUP_SUBOBJECT& operator=(CD3DX12_HIT_GROUP_SUBOBJECT&& other) = default;
    void SetHitGroupExport(LPCWSTR exportName)
    {
        m_Desc.HitGroupExport = m_Strings[0].LocalCopy(exportName, true);
    }
    void SetHitGroupType(D3D12_HIT_GROUP_TYPE Type) noexcept { m_Desc.Type = Type; }
    void SetAnyHitShaderImport(LPCWSTR importName)
    {
        m_Desc.AnyHitShaderImport = m_Strings[1].LocalCopy(importName, true);
    }
    void SetClosestHitShaderImport(LPCWSTR importName)
    {
        m_Desc.ClosestHitShaderImport = m_Strings[2].LocalCopy(importName, true);
    }
    void SetIntersectionShaderImport(LPCWSTR importName)
    {
        m_Desc.IntersectionShaderImport = m_Strings[3].LocalCopy(importName, true);
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
    }
    operator const D3D12_HIT_GROUP_DESC&() const noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        for (UINT i = 0; i < m_NumStrings; i++)
        {
            m_Strings[i].clear();
        }
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_HIT_GROUP_DESC m_Desc;
    static constexpr UINT m_NumStrings = 4;
    CD3DX12_STATE_OBJECT_DESC::StringContainer<LPCWSTR, std::wstring>
        m_Strings[m_NumStrings]; // one string for every entrypoint name
};

//------------------------------------------------------------------------------------------------
class CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT() noexcept
        : m_Desc({})
    {
        Init();
    }
    CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc({})
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT(const D3D12_RAYTRACING_SHADER_CONFIG &desc)
        : m_Desc(desc)
    {
        Init();
    }
    CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT(const D3D12_RAYTRACING_SHADER_CONFIG &desc, CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(desc)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT(const CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT& other) = delete;
    CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT& operator=(const CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT& other) = delete;
    CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT(CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT&& other) = default;
    CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT& operator=(CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT&& other) = default;
    void Config(UINT MaxPayloadSizeInBytes, UINT MaxAttributeSizeInBytes) noexcept
    {
        m_Desc.MaxPayloadSizeInBytes = MaxPayloadSizeInBytes;
        m_Desc.MaxAttributeSizeInBytes = MaxAttributeSizeInBytes;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
    }
    operator const D3D12_RAYTRACING_SHADER_CONFIG&() const noexcept { return m_Desc; }
    operator D3D12_RAYTRACING_SHADER_CONFIG&() noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_RAYTRACING_SHADER_CONFIG m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT() noexcept
        : m_Desc({})
    {
        Init();
    }
    CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc({})
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT(const D3D12_RAYTRACING_PIPELINE_CONFIG &desc)
        : m_Desc(desc)
    {
        Init();
    }
    CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT(const D3D12_RAYTRACING_PIPELINE_CONFIG &desc, CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(desc)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT(const CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT& other) = delete;
    CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT& operator=(const CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT& other) = delete;
    CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT(CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT&& other) = default;
    CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT& operator=(CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT&& other) = default;
    void Config(UINT MaxTraceRecursionDepth) noexcept
    {
        m_Desc.MaxTraceRecursionDepth = MaxTraceRecursionDepth;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
    }
    operator const D3D12_RAYTRACING_PIPELINE_CONFIG&() const noexcept { return m_Desc; }
    operator D3D12_RAYTRACING_PIPELINE_CONFIG&() noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_RAYTRACING_PIPELINE_CONFIG m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT() noexcept
        : m_Desc({})
    {
        Init();
    }
    CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc({})
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT(const D3D12_RAYTRACING_PIPELINE_CONFIG1 &desc)
        : m_Desc(desc)
    {
        Init();
    }
    CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT(const D3D12_RAYTRACING_PIPELINE_CONFIG1 &desc, CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(desc)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT(const CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT& other) = delete;
    CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT& operator=(const CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT& other) = delete;
    CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT(CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT&& other) = default;
    CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT& operator=(CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT&& other) = default;
    void Config(UINT MaxTraceRecursionDepth, D3D12_RAYTRACING_PIPELINE_FLAGS Flags) noexcept
    {
        m_Desc.MaxTraceRecursionDepth = MaxTraceRecursionDepth;
        m_Desc.Flags = Flags;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG1;
    }
    operator const D3D12_RAYTRACING_PIPELINE_CONFIG1&() const noexcept { return m_Desc; }
    operator D3D12_RAYTRACING_PIPELINE_CONFIG1&() noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_RAYTRACING_PIPELINE_CONFIG1 m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT(const CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT& other) = delete;
    CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT& operator=(const CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT& other) = delete;
    CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT(CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT&& other) = default;
    CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT& operator=(CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT&& other) = default;
    void SetRootSignature(ID3D12RootSignature* pRootSig) noexcept
    {
        m_pRootSig = pRootSig;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    }
    operator ID3D12RootSignature*() const noexcept { return D3DX12_COM_PTR_GET(m_pRootSig); }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_pRootSig = nullptr;
    }
    void* Data() noexcept override { return D3DX12_COM_PTR_ADDRESSOF(m_pRootSig); }
    D3DX12_COM_PTR<ID3D12RootSignature> m_pRootSig;
};


class CD3DX12_API_EXTENSION_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_API_EXTENSION_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_API_EXTENSION_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetApiExtension(ID3D12Extension* pExtension) noexcept
    {
        m_pExtension = pExtension;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_API_EXTENSION;
    }
    operator ID3D12Extension*() const noexcept { return D3DX12_COM_PTR_GET(m_pExtension); }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_pExtension = nullptr;
    }
    void* Data() noexcept override { return D3DX12_COM_PTR_ADDRESSOF(m_pExtension); }
    D3DX12_COM_PTR<ID3D12Extension> m_pExtension;
};


//------------------------------------------------------------------------------------------------
class CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT(const CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT& other) = delete;
    CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT& operator=(const CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT& other) = delete;
    CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT(CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT&& other) = default;
    CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT& operator=(CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT&& other) = default;
    void SetRootSignature(ID3D12RootSignature* pRootSig) noexcept
    {
        m_pRootSig = pRootSig;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
    }
    operator ID3D12RootSignature*() const noexcept { return D3DX12_COM_PTR_GET(m_pRootSig); }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_pRootSig = nullptr;
    }
    void* Data() noexcept override { return D3DX12_COM_PTR_ADDRESSOF(m_pRootSig); }
    D3DX12_COM_PTR<ID3D12RootSignature> m_pRootSig;
};

#if defined(D3D12_SDK_VERSION) && (D3D12_SDK_VERSION >= 618)
//------------------------------------------------------------------------------------------------
class CD3DX12_GLOBAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_GLOBAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT() noexcept
        : m_Desc({})
    {
        Init();
    }
    CD3DX12_GLOBAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc({})
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_GLOBAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT(const CD3DX12_GLOBAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT& other) = delete;
    CD3DX12_GLOBAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT& operator=(const CD3DX12_GLOBAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT& other) = delete;
    CD3DX12_GLOBAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT(CD3DX12_GLOBAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT&& other) = default;
    CD3DX12_GLOBAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT& operator=(CD3DX12_GLOBAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT&& other) = default;
    void SetRootSignature(const D3D12_SERIALIZED_ROOT_SIGNATURE_DESC* pDesc) noexcept
    {
        if (pDesc)
        {
            m_Desc.Desc = {};
            m_Desc.Desc.pSerializedBlob = pDesc->pSerializedBlob;
            m_Desc.Desc.SerializedBlobSizeInBytes = pDesc->SerializedBlobSizeInBytes;
        }
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_SERIALIZED_ROOT_SIGNATURE;
    }
    operator const D3D12_GLOBAL_SERIALIZED_ROOT_SIGNATURE&() const noexcept { return m_Desc; }
    operator D3D12_GLOBAL_SERIALIZED_ROOT_SIGNATURE&() noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_GLOBAL_SERIALIZED_ROOT_SIGNATURE m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_LOCAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_LOCAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT() noexcept
        : m_Desc({})
    {
        Init();
    }
    CD3DX12_LOCAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc({})
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_LOCAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT(const CD3DX12_LOCAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT& other) = delete;
    CD3DX12_LOCAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT& operator=(const CD3DX12_LOCAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT& other) = delete;
    CD3DX12_LOCAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT(CD3DX12_LOCAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT&& other) = default;
    CD3DX12_LOCAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT& operator=(CD3DX12_LOCAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT&& other) = default;
    void SetRootSignature(const D3D12_SERIALIZED_ROOT_SIGNATURE_DESC* pDesc) noexcept
    {
        if (pDesc)
        {
            m_Desc.Desc = {};
            m_Desc.Desc.pSerializedBlob = pDesc->pSerializedBlob;
            m_Desc.Desc.SerializedBlobSizeInBytes = pDesc->SerializedBlobSizeInBytes;
        }
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_LOCAL_SERIALIZED_ROOT_SIGNATURE;
    }
    operator const D3D12_LOCAL_SERIALIZED_ROOT_SIGNATURE&() const noexcept { return m_Desc; }
    operator D3D12_LOCAL_SERIALIZED_ROOT_SIGNATURE&() noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_LOCAL_SERIALIZED_ROOT_SIGNATURE m_Desc;
};


//------------------------------------------------------------------------------------------------
class CD3DX12_EXISTING_COLLECTION_BY_KEY_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_EXISTING_COLLECTION_BY_KEY_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_EXISTING_COLLECTION_BY_KEY_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_EXISTING_COLLECTION_BY_KEY_SUBOBJECT(const CD3DX12_EXISTING_COLLECTION_BY_KEY_SUBOBJECT& other) = delete;
    CD3DX12_EXISTING_COLLECTION_BY_KEY_SUBOBJECT& operator=(const CD3DX12_EXISTING_COLLECTION_BY_KEY_SUBOBJECT& other) = delete;
    CD3DX12_EXISTING_COLLECTION_BY_KEY_SUBOBJECT(CD3DX12_EXISTING_COLLECTION_BY_KEY_SUBOBJECT&& other) = default;
    CD3DX12_EXISTING_COLLECTION_BY_KEY_SUBOBJECT& operator=(CD3DX12_EXISTING_COLLECTION_BY_KEY_SUBOBJECT&& other) = default;
    void SetExistingCollection(const void* pKey, UINT KeySize) noexcept
    {
        const unsigned char* pKeyBytes = static_cast<const unsigned char *>(pKey);
        m_Key.clear();
        m_Key.insert(m_Key.begin(), pKeyBytes, pKeyBytes + KeySize);
        m_Desc.pKey = m_Key.data();
        m_Desc.KeySize = KeySize;
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
        return D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION_BY_KEY;
    }
    operator const D3D12_EXISTING_COLLECTION_BY_KEY_DESC&() const noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        m_Strings.clear();
        m_Exports.clear();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_EXISTING_COLLECTION_BY_KEY_DESC m_Desc;
    std::vector<unsigned char> m_Key;
    CD3DX12_STATE_OBJECT_DESC::StringContainer<LPCWSTR, std::wstring> m_Strings;
    std::vector<D3D12_EXPORT_DESC> m_Exports;
};

#endif // defined(D3D12_SDK_VERSION) && (D3D12_SDK_VERSION >= 618)

//------------------------------------------------------------------------------------------------
class CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT() noexcept
        : m_Desc({})
    {
        Init();
    }
    CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc({})
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT(const D3D12_STATE_OBJECT_CONFIG &desc) noexcept
        : m_Desc(desc)
    {
        Init();
    }
    CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT(const D3D12_STATE_OBJECT_CONFIG &desc, CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(desc)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT(const CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT& other) = delete;
    CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT& operator=(const CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT& other) = delete;
    CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT(CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT&& other) = default;
    CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT& operator=(CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT&& other) = default;
    void SetFlags(D3D12_STATE_OBJECT_FLAGS Flags) noexcept
    {
        m_Desc.Flags = Flags;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_STATE_OBJECT_CONFIG;
    }
    operator const D3D12_STATE_OBJECT_CONFIG&() const noexcept { return m_Desc; }
    operator D3D12_STATE_OBJECT_CONFIG&() noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_STATE_OBJECT_CONFIG m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_NODE_MASK_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_NODE_MASK_SUBOBJECT() noexcept
        : m_Desc({})
    {
        Init();
    }
    CD3DX12_NODE_MASK_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc({})
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_NODE_MASK_SUBOBJECT(const D3D12_NODE_MASK &desc) noexcept
        : m_Desc(desc)
    {
        Init();
    }
    CD3DX12_NODE_MASK_SUBOBJECT(const D3D12_NODE_MASK &desc, CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(desc)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_NODE_MASK_SUBOBJECT(const CD3DX12_NODE_MASK_SUBOBJECT& other) = delete;
    CD3DX12_NODE_MASK_SUBOBJECT& operator=(const CD3DX12_NODE_MASK_SUBOBJECT& other) = delete;
    CD3DX12_NODE_MASK_SUBOBJECT(CD3DX12_NODE_MASK_SUBOBJECT&& other) = default;
    CD3DX12_NODE_MASK_SUBOBJECT& operator=(CD3DX12_NODE_MASK_SUBOBJECT&& other) = default;
    void SetNodeMask(UINT NodeMask) noexcept
    {
        m_Desc.NodeMask = NodeMask;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK;
    }
    operator const D3D12_NODE_MASK&() const noexcept { return m_Desc; }
    operator D3D12_NODE_MASK&() noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_NODE_MASK m_Desc;
};

#if defined(D3D12_SDK_VERSION) && (D3D12_SDK_VERSION >= 612)
//------------------------------------------------------------------------------------------------
class CD3DX12_STREAM_OUTPUT_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_STREAM_OUTPUT_SUBOBJECT()
    {
        Init();
    }
    CD3DX12_STREAM_OUTPUT_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void AddSODeclEntry(const D3D12_SO_DECLARATION_ENTRY &entry)
    {
        m_soDecalEntries.emplace_back(D3D12_SO_DECLARATION_ENTRY{
            entry.Stream,
            m_Strings.LocalCopy(entry.SemanticName),
            entry.SemanticIndex,
            entry.StartComponent,
            entry.ComponentCount,
            entry.OutputSlot
        });
        m_Desc.NumEntries++;
        // Below: using ugly way to get pointer in case .data() is not defined
        m_Desc.pSODeclaration = &m_soDecalEntries[0];
    }
    void SetSODeclEntries(const D3D12_SO_DECLARATION_ENTRY* soDeclEntries, UINT numEntries)
    {
        m_soDecalEntries.resize(numEntries);
        for (UINT i = 0; i < numEntries; i++)
        {
            m_soDecalEntries[i] = D3D12_SO_DECLARATION_ENTRY{
                soDeclEntries[i].Stream,
                m_Strings.LocalCopy(soDeclEntries[i].SemanticName),
                soDeclEntries[i].SemanticIndex,
                soDeclEntries[i].StartComponent,
                soDeclEntries[i].ComponentCount,
                soDeclEntries[i].OutputSlot
            };
        }
        m_Desc.NumEntries = numEntries;
        // Below: using ugly way to get pointer in case .data() is not defined
        if (numEntries > 0)
        {
            m_Desc.pSODeclaration = &m_soDecalEntries[0];
        }
    }
    void SetBufferStrides(const UINT* bufferStrides, UINT numStrides)
    {
        for (UINT i = 0; i < numStrides; ++i)
        {
            m_Strides[i] = bufferStrides[i];
        }
        m_Desc.pBufferStrides = m_Strides;
        m_Desc.NumStrides = numStrides;
    }
    void SetRasterizedStream(UINT rasterizedStream)
    {
        m_Desc.RasterizedStream = rasterizedStream;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_STREAM_OUTPUT;
    }
    operator const D3D12_STREAM_OUTPUT_DESC& () const noexcept { return m_Desc; }

private:
    void Init()
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_STREAM_OUTPUT_DESC m_Desc;
    CD3DX12_STATE_OBJECT_DESC::StringContainer<LPCSTR, std::string> m_Strings;
    std::vector<D3D12_SO_DECLARATION_ENTRY> m_soDecalEntries;
    UINT m_Strides[D3D12_SO_STREAM_COUNT];
};

//------------------------------------------------------------------------------------------------
class CD3DX12_BLEND_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_BLEND_SUBOBJECT()
        : m_Desc(CD3DX12_BLEND_DESC(D3D12_DEFAULT))
    {
        Init();
    }
    CD3DX12_BLEND_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(CD3DX12_BLEND_DESC(D3D12_DEFAULT))
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_BLEND_SUBOBJECT(const D3D12_BLEND_DESC &desc)
        : m_Desc(desc)
    {
        Init();
    }
    CD3DX12_BLEND_SUBOBJECT(const D3D12_BLEND_DESC &desc, CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(desc)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetAlphaToCoverageEnable(bool alphaToCoverageEnable)
    {
        m_Desc.AlphaToCoverageEnable = alphaToCoverageEnable;
    }
    void SetIndependentBlendEnable(bool independentBlendEnable)
    {
        m_Desc.IndependentBlendEnable = independentBlendEnable;
    }
    void SetRenderTarget(UINT renderTargetIndex, const D3D12_RENDER_TARGET_BLEND_DESC& renderTargetBlendDesc)
    {
        m_Desc.RenderTarget[renderTargetIndex].BlendEnable = renderTargetBlendDesc.BlendEnable;
        m_Desc.RenderTarget[renderTargetIndex].BlendOp = renderTargetBlendDesc.BlendOp;
        m_Desc.RenderTarget[renderTargetIndex].BlendOpAlpha = renderTargetBlendDesc.BlendOpAlpha;
        m_Desc.RenderTarget[renderTargetIndex].DestBlend = renderTargetBlendDesc.DestBlend;
        m_Desc.RenderTarget[renderTargetIndex].DestBlendAlpha = renderTargetBlendDesc.DestBlendAlpha;
        m_Desc.RenderTarget[renderTargetIndex].LogicOp = renderTargetBlendDesc.LogicOp;
        m_Desc.RenderTarget[renderTargetIndex].LogicOpEnable = renderTargetBlendDesc.LogicOpEnable;
        m_Desc.RenderTarget[renderTargetIndex].RenderTargetWriteMask = renderTargetBlendDesc.RenderTargetWriteMask;
        m_Desc.RenderTarget[renderTargetIndex].SrcBlend = renderTargetBlendDesc.SrcBlend;
        m_Desc.RenderTarget[renderTargetIndex].SrcBlendAlpha = renderTargetBlendDesc.SrcBlendAlpha;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_BLEND;
    }
    operator const D3D12_BLEND_DESC& () const noexcept { return m_Desc; }
    operator D3D12_BLEND_DESC& () noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
    }
    void* Data() noexcept override { return &m_Desc; }
    CD3DX12_BLEND_DESC m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_RASTERIZER_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_RASTERIZER_SUBOBJECT()
        : m_Desc(CD3DX12_RASTERIZER_DESC2(D3D12_DEFAULT))
    {
        Init();
    }
    CD3DX12_RASTERIZER_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(CD3DX12_RASTERIZER_DESC2(D3D12_DEFAULT))
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_RASTERIZER_SUBOBJECT(const D3D12_RASTERIZER_DESC2 &desc)
        : m_Desc(desc)
    {
        Init();
    }
    CD3DX12_RASTERIZER_SUBOBJECT(const D3D12_RASTERIZER_DESC2 &desc, CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(desc)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetFillMode(D3D12_FILL_MODE fillMode)
    {
        m_Desc.FillMode = fillMode;
    }
    void SetCullMode(D3D12_CULL_MODE cullMode)
    {
        m_Desc.CullMode = cullMode;
    }
    void SetFrontCounterClockwise(BOOL frontCounterClockwise)
    {
        m_Desc.FrontCounterClockwise = frontCounterClockwise;
    }
    void SetDepthBias(FLOAT depthBias)
    {
        m_Desc.DepthBias = depthBias;
    }
    void SetDepthBiasClamp(FLOAT depthBiasClamp)
    {
        m_Desc.DepthBiasClamp = depthBiasClamp;
    }
    void SetSlopeScaledDepthBias(FLOAT slopeScaledDepthBias)
    {
        m_Desc.SlopeScaledDepthBias = slopeScaledDepthBias;
    }
    void SetDepthClipEnable(BOOL depthClipEnable)
    {
        m_Desc.DepthClipEnable = depthClipEnable;
    }
    void SetLineRasterizationMode(D3D12_LINE_RASTERIZATION_MODE lineRasterizationMode)
    {
        m_Desc.LineRasterizationMode = lineRasterizationMode;
    }
    void SetForcedSampleCount(UINT forcedSampleCount)
    {
        m_Desc.ForcedSampleCount = forcedSampleCount;
    }
    void SetConservativeRaster(D3D12_CONSERVATIVE_RASTERIZATION_MODE conservativeRaster)
    {
        m_Desc.ConservativeRaster = conservativeRaster;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_RASTERIZER;
    }
    operator const D3D12_RASTERIZER_DESC2& () const noexcept { return m_Desc; }
    operator D3D12_RASTERIZER_DESC2& () noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
    }
    void* Data() noexcept override { return &m_Desc; }
    CD3DX12_RASTERIZER_DESC2 m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_DEPTH_STENCIL2_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_DEPTH_STENCIL2_SUBOBJECT()
        : m_Desc(CD3DX12_DEPTH_STENCIL_DESC2(D3D12_DEFAULT))
    {
        Init();
    }
    CD3DX12_DEPTH_STENCIL2_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(CD3DX12_DEPTH_STENCIL_DESC2(D3D12_DEFAULT))
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_DEPTH_STENCIL2_SUBOBJECT(const D3D12_DEPTH_STENCIL_DESC2 &desc)
        : m_Desc(desc)
    {
        Init();
    }
    CD3DX12_DEPTH_STENCIL2_SUBOBJECT(const D3D12_DEPTH_STENCIL_DESC2 &desc, CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(desc)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetDepthEnable(BOOL depthEnable)
    {
        m_Desc.DepthEnable = depthEnable;
    }
    void SetDepthWriteMask(D3D12_DEPTH_WRITE_MASK depthWriteMask)
    {
        m_Desc.DepthWriteMask = depthWriteMask;
    }
    void SetDepthFunc(D3D12_COMPARISON_FUNC depthFunc)
    {
        m_Desc.DepthFunc = depthFunc;
    }
    void SetStencilEnable(BOOL stencilEnable)
    {
        m_Desc.StencilEnable = stencilEnable;
    }
    void SetFrontFace(D3D12_DEPTH_STENCILOP_DESC1 frontFace)
    {
        m_Desc.FrontFace = {
            frontFace.StencilFailOp,
            frontFace.StencilDepthFailOp,
            frontFace.StencilPassOp,
            frontFace.StencilFunc,
            frontFace.StencilReadMask,
            frontFace.StencilWriteMask
        };
    }
    void SetBackFace(D3D12_DEPTH_STENCILOP_DESC1 backFace)
    {
        m_Desc.BackFace = {
            backFace.StencilFailOp,
            backFace.StencilDepthFailOp,
            backFace.StencilPassOp,
            backFace.StencilFunc,
            backFace.StencilReadMask,
            backFace.StencilWriteMask
        };
    }
    void SetDepthBoundsTestEnable(BOOL depthBoundsTestEnable)
    {
        m_Desc.DepthBoundsTestEnable = depthBoundsTestEnable;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL2;
    }
    operator const D3D12_DEPTH_STENCIL_DESC2& () const noexcept { return m_Desc; }
    operator D3D12_DEPTH_STENCIL_DESC2& () noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
    }
    void* Data() noexcept override { return &m_Desc; }
    CD3DX12_DEPTH_STENCIL_DESC2 m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_INPUT_LAYOUT_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_INPUT_LAYOUT_SUBOBJECT()
    {
        Init();
    }
    CD3DX12_INPUT_LAYOUT_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void AddInputLayoutElementDesc(D3D12_INPUT_ELEMENT_DESC inputLayoutElementDesc)
    {
        m_inputLayoutElements.emplace_back(
            D3D12_INPUT_ELEMENT_DESC{
                m_Strings.LocalCopy(inputLayoutElementDesc.SemanticName),
                inputLayoutElementDesc.SemanticIndex,
                inputLayoutElementDesc.Format,
                inputLayoutElementDesc.InputSlot,
                inputLayoutElementDesc.AlignedByteOffset,
                inputLayoutElementDesc.InputSlotClass,
                inputLayoutElementDesc.InstanceDataStepRate
            });
        ++m_Desc.NumElements;
        // Below: using ugly way to get pointer in case .data() is not defined
        m_Desc.pInputElementDescs = &m_inputLayoutElements[0];
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT;
    }
    operator const D3D12_INPUT_LAYOUT_DESC& () const noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        m_inputLayoutElements.clear();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_INPUT_LAYOUT_DESC m_Desc;
    std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayoutElements;
    CD3DX12_STATE_OBJECT_DESC::StringContainer<LPCSTR, std::string> m_Strings;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_IB_STRIP_CUT_VALUE_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_IB_STRIP_CUT_VALUE_SUBOBJECT()
        : m_Desc(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED)
    {
        Init();
    }
    CD3DX12_IB_STRIP_CUT_VALUE_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_IB_STRIP_CUT_VALUE_SUBOBJECT(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE desc)
        : m_Desc(desc)
    {
        Init();
    }
    CD3DX12_IB_STRIP_CUT_VALUE_SUBOBJECT(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE desc, CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(desc)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetIBStripCutValue(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE ibStripCutValue)
    {
        m_Desc = ibStripCutValue;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_IB_STRIP_CUT_VALUE;
    }
    operator const D3D12_INDEX_BUFFER_STRIP_CUT_VALUE& () const noexcept { return m_Desc; }
    operator D3D12_INDEX_BUFFER_STRIP_CUT_VALUE& () noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_INDEX_BUFFER_STRIP_CUT_VALUE m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_PRIMITIVE_TOPOLOGY_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_PRIMITIVE_TOPOLOGY_SUBOBJECT()
        : m_Desc(D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED)
    {
        Init();
    }
    CD3DX12_PRIMITIVE_TOPOLOGY_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_PRIMITIVE_TOPOLOGY_SUBOBJECT(D3D12_PRIMITIVE_TOPOLOGY_TYPE desc)
        : m_Desc(desc)
    {
        Init();
    }
    CD3DX12_PRIMITIVE_TOPOLOGY_SUBOBJECT(D3D12_PRIMITIVE_TOPOLOGY_TYPE desc, CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(desc)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologytype)
    {
        m_Desc = primitiveTopologytype;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY;
    }
    operator const D3D12_PRIMITIVE_TOPOLOGY_TYPE& () const noexcept { return m_Desc; }
    operator D3D12_PRIMITIVE_TOPOLOGY_TYPE& () noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_PRIMITIVE_TOPOLOGY_TYPE m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_RENDER_TARGET_FORMATS_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_RENDER_TARGET_FORMATS_SUBOBJECT()
        : m_Desc({})
    {
        Init();
    }
    CD3DX12_RENDER_TARGET_FORMATS_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc({})
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_RENDER_TARGET_FORMATS_SUBOBJECT(const D3D12_RT_FORMAT_ARRAY &desc)
        : m_Desc(desc)
    {
        Init();
    }
    CD3DX12_RENDER_TARGET_FORMATS_SUBOBJECT(const D3D12_RT_FORMAT_ARRAY &desc, CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(desc)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetNumRenderTargets(UINT numRenderTargets)
    {
        m_Desc.NumRenderTargets = numRenderTargets;
    }
    void SetRenderTargetFormat(UINT renderTarget, DXGI_FORMAT renderTargetFormat)
    {
        m_Desc.RTFormats[renderTarget] = renderTargetFormat;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS;
    }
    operator const D3D12_RT_FORMAT_ARRAY& () const noexcept { return m_Desc; }
    operator D3D12_RT_FORMAT_ARRAY& () noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_RT_FORMAT_ARRAY m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_DEPTH_STENCIL_FORMAT_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_DEPTH_STENCIL_FORMAT_SUBOBJECT()
        : m_Desc(DXGI_FORMAT_UNKNOWN)
    {
        Init();
    }
    CD3DX12_DEPTH_STENCIL_FORMAT_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(DXGI_FORMAT_UNKNOWN)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_DEPTH_STENCIL_FORMAT_SUBOBJECT(DXGI_FORMAT desc)
        : m_Desc(desc)
    {
        Init();
    }
    CD3DX12_DEPTH_STENCIL_FORMAT_SUBOBJECT(DXGI_FORMAT desc, CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(desc)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetDepthStencilFormat(DXGI_FORMAT depthStencilFormat)
    {
        m_Desc = depthStencilFormat;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT;
    }
    operator const DXGI_FORMAT& () const noexcept { return m_Desc; }
    operator DXGI_FORMAT& () noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
    }
    void* Data() noexcept override { return &m_Desc; }
    DXGI_FORMAT m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_SAMPLE_DESC_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_SAMPLE_DESC_SUBOBJECT()
        : m_Desc({1, 0})
    {
        Init();
    }
    CD3DX12_SAMPLE_DESC_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc({1, 0})
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_SAMPLE_DESC_SUBOBJECT(const DXGI_SAMPLE_DESC &desc)
        : m_Desc(desc)
    {
        Init();
    }
    CD3DX12_SAMPLE_DESC_SUBOBJECT(const DXGI_SAMPLE_DESC &desc, CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(desc)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetCount(UINT count)
    {
        m_Desc.Count = count;
    }
    void SetQuality(UINT quality)
    {
        m_Desc.Quality = quality;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_SAMPLE_DESC;
    }
    operator const DXGI_SAMPLE_DESC& () const noexcept { return m_Desc; }
    operator DXGI_SAMPLE_DESC& () noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
    }
    void* Data() noexcept override { return &m_Desc; }
    DXGI_SAMPLE_DESC m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_FLAGS_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_FLAGS_SUBOBJECT()
        : m_Desc(D3D12_PIPELINE_STATE_FLAG_NONE)
    {
        Init();
    }
    CD3DX12_FLAGS_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(D3D12_PIPELINE_STATE_FLAG_NONE)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_FLAGS_SUBOBJECT(D3D12_PIPELINE_STATE_FLAGS desc)
        : m_Desc(desc)
    {
        Init();
    }
    CD3DX12_FLAGS_SUBOBJECT(D3D12_PIPELINE_STATE_FLAGS desc, CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(desc)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetFlags(D3D12_PIPELINE_STATE_FLAGS flags)
    {
        m_Desc = flags;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_FLAGS;
    }
    operator const D3D12_PIPELINE_STATE_FLAGS& () const noexcept { return m_Desc; }
    operator D3D12_PIPELINE_STATE_FLAGS& () noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_PIPELINE_STATE_FLAGS m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_VIEW_INSTANCING_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_VIEW_INSTANCING_SUBOBJECT()
    {
        Init();
    }
    CD3DX12_VIEW_INSTANCING_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void AddViewInstanceLocation(D3D12_VIEW_INSTANCE_LOCATION viewInstanceLocation)
    {
        m_Desc.ViewInstanceCount++;
        m_viewInstanceLocations.emplace_back(
            D3D12_VIEW_INSTANCE_LOCATION
            {
                viewInstanceLocation.ViewportArrayIndex,
                viewInstanceLocation.RenderTargetArrayIndex
            }
        );
        // Below: using ugly way to get pointer in case .data() is not defined
        m_Desc.pViewInstanceLocations = &m_viewInstanceLocations[0];
    }
    void SetFlags(D3D12_VIEW_INSTANCING_FLAGS flags)
    {
        m_Desc.Flags = flags;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_VIEW_INSTANCING;
    }
    operator const D3D12_VIEW_INSTANCING_DESC& () const noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = CD3DX12_VIEW_INSTANCING_DESC(D3D12_DEFAULT);
        m_viewInstanceLocations.clear();
    }
    void* Data() noexcept override { return &m_Desc; }
    CD3DX12_VIEW_INSTANCING_DESC m_Desc;
    std::vector<D3D12_VIEW_INSTANCE_LOCATION> m_viewInstanceLocations;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_DEPTH_STENCIL_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_DEPTH_STENCIL_SUBOBJECT()
        : m_Desc(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT))
    {
        Init();
    }
    CD3DX12_DEPTH_STENCIL_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT))
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_DEPTH_STENCIL_SUBOBJECT(const D3D12_DEPTH_STENCIL_DESC &desc)
        : m_Desc(desc)
    {
        Init();
    }
    CD3DX12_DEPTH_STENCIL_SUBOBJECT(const D3D12_DEPTH_STENCIL_DESC &desc, CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(desc)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetDepthEnable(BOOL depthEnable)
    {
        m_Desc.DepthEnable = depthEnable;
    }
    void SetDepthWriteMask(D3D12_DEPTH_WRITE_MASK depthWriteMask)
    {
        m_Desc.DepthWriteMask = depthWriteMask;
    }
    void SetDepthFunc(D3D12_COMPARISON_FUNC depthFunc)
    {
        m_Desc.DepthFunc = depthFunc;
    }
    void SetStencilEnable(BOOL stencilEnable)
    {
        m_Desc.StencilEnable = stencilEnable;
    }
    void SetStencilReadMask(UINT8 stencilReadMask)
    {
        m_Desc.StencilReadMask = stencilReadMask;
    }
    void SetStencilWriteMask(UINT8 stencilWriteMask)
    {
        m_Desc.StencilWriteMask = stencilWriteMask;
    }
    void SetFrontFace(D3D12_DEPTH_STENCILOP_DESC frontFace)
    {
        m_Desc.FrontFace = {
            frontFace.StencilFailOp,
            frontFace.StencilDepthFailOp,
            frontFace.StencilPassOp,
            frontFace.StencilFunc
        };
    }
    void SetBackFace(D3D12_DEPTH_STENCILOP_DESC backFace)
    {
        m_Desc.BackFace = {
            backFace.StencilFailOp,
            backFace.StencilDepthFailOp,
            backFace.StencilPassOp,
            backFace.StencilFunc
        };
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL;
    }
    operator const D3D12_DEPTH_STENCIL_DESC& () const noexcept { return m_Desc; }
    operator D3D12_DEPTH_STENCIL_DESC& () noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
    }
    void* Data() noexcept override { return &m_Desc; }
    CD3DX12_DEPTH_STENCIL_DESC m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_DEPTH_STENCIL1_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_DEPTH_STENCIL1_SUBOBJECT()
        : m_Desc(CD3DX12_DEPTH_STENCIL_DESC1(D3D12_DEFAULT))
    {
        Init();
    }
    CD3DX12_DEPTH_STENCIL1_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(CD3DX12_DEPTH_STENCIL_DESC1(D3D12_DEFAULT))
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_DEPTH_STENCIL1_SUBOBJECT(const D3D12_DEPTH_STENCIL_DESC1 &desc)
        : m_Desc(desc)
    {
        Init();
    }
    CD3DX12_DEPTH_STENCIL1_SUBOBJECT(const D3D12_DEPTH_STENCIL_DESC1 &desc, CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(desc)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetDepthEnable(BOOL depthEnable)
    {
        m_Desc.DepthEnable = depthEnable;
    }
    void SetDepthWriteMask(D3D12_DEPTH_WRITE_MASK depthWriteMask)
    {
        m_Desc.DepthWriteMask = depthWriteMask;
    }
    void SetDepthFunc(D3D12_COMPARISON_FUNC depthFunc)
    {
        m_Desc.DepthFunc = depthFunc;
    }
    void SetStencilEnable(BOOL stencilEnable)
    {
        m_Desc.StencilEnable = stencilEnable;
    }
    void SetStencilReadMask(UINT8 stencilReadMask)
    {
        m_Desc.StencilReadMask = stencilReadMask;
    }
    void SetStencilWriteMask(UINT8 stencilWriteMask)
    {
        m_Desc.StencilWriteMask = stencilWriteMask;
    }
    void SetFrontFace(D3D12_DEPTH_STENCILOP_DESC frontFace)
    {
        m_Desc.FrontFace = {
            frontFace.StencilFailOp,
            frontFace.StencilDepthFailOp,
            frontFace.StencilPassOp,
            frontFace.StencilFunc
        };
    }
    void SetBackFace(D3D12_DEPTH_STENCILOP_DESC backFace)
    {
        m_Desc.BackFace = {
            backFace.StencilFailOp,
            backFace.StencilDepthFailOp,
            backFace.StencilPassOp,
            backFace.StencilFunc
        };
    }
    void SetDepthBoundsTestEnable(BOOL depthBoundsTestEnable)
    {
        m_Desc.DepthBoundsTestEnable = depthBoundsTestEnable;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL1;
    }
    operator const D3D12_DEPTH_STENCIL_DESC1& () const noexcept { return m_Desc; }
    operator D3D12_DEPTH_STENCIL_DESC1& () noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
    }
    void* Data() noexcept override { return &m_Desc; }
    CD3DX12_DEPTH_STENCIL_DESC1 m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_SAMPLE_MASK_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_SAMPLE_MASK_SUBOBJECT()
        : m_Desc(0xffffffffu)
    {
        Init();
    }
    CD3DX12_SAMPLE_MASK_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(0xffffffffu)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    CD3DX12_SAMPLE_MASK_SUBOBJECT(UINT desc)
        : m_Desc(desc)
    {
        Init();
    }
    CD3DX12_SAMPLE_MASK_SUBOBJECT(UINT desc, CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        : m_Desc(desc)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetSampleMask(UINT sampleMask)
    {
        m_Desc = sampleMask;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_SAMPLE_MASK;
    }
    operator const UINT& () const noexcept { return m_Desc; }
    operator UINT& () noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
    }
    void* Data() noexcept override { return &m_Desc; }
    UINT m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_GENERIC_PROGRAM_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_GENERIC_PROGRAM_SUBOBJECT()
    {
        Init();
    }
    CD3DX12_GENERIC_PROGRAM_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetProgramName(LPCWSTR ProgramName)
    {
        m_Desc.ProgramName = m_Strings.LocalCopy(ProgramName);
    }
    void AddExport(LPCWSTR exportName)
    {
        m_Exports.emplace_back(m_Strings.LocalCopy(exportName));
        m_Desc.NumExports++;
        // Below: using ugly way to get pointer in case .data() is not defined
        m_Desc.pExports = &m_Exports[0];
    }
    void AddSubobject(const D3D12_STATE_SUBOBJECT& subobject)
    {
        m_Subobjects.emplace_back(&subobject);
        m_Desc.NumSubobjects++;
        // Below: using ugly way to get pointer in case .data() is not defined
        m_Desc.ppSubobjects = &m_Subobjects[0];
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_GENERIC_PROGRAM;
    }
    operator const D3D12_GENERIC_PROGRAM_DESC& () const noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_GENERIC_PROGRAM_DESC m_Desc;
    std::vector<LPCWSTR> m_Exports;
    std::vector<D3D12_STATE_SUBOBJECT const*> m_Subobjects;
    CD3DX12_STATE_OBJECT_DESC::StringContainer<LPCWSTR, std::wstring> m_Strings;
};


//------------------------------------------------------------------------------------------------
class CD3DX12_NODE_OUTPUT_OVERRIDES
{
public:
    CD3DX12_NODE_OUTPUT_OVERRIDES(const D3D12_NODE_OUTPUT_OVERRIDES** ppOwner, UINT* pNumOutputOverrides) noexcept
    {
        m_Desc.clear();
        m_ppOwner = ppOwner;
        *m_ppOwner = nullptr;
        m_pNumOutputOverrides = pNumOutputOverrides;
        *m_pNumOutputOverrides = 0;
    }
    void NewOutputOverride()
    {
        m_Desc.emplace_back(D3D12_NODE_OUTPUT_OVERRIDES{});
        *m_ppOwner = m_Desc.data();
        (*m_pNumOutputOverrides)++;
    }
    void OutputIndex(UINT index)
    {
        m_Desc.back().OutputIndex = index;
    }
    void NewName(LPCWSTR Name, UINT ArrayIndex = 0)
    {
        m_NodeIDs.emplace_front(D3D12_NODE_ID{ m_Strings.LocalCopy(Name),ArrayIndex });
        m_Desc.back().pNewName = &m_NodeIDs.front();
    }
    void AllowSparseNodes(BOOL bAllow)
    {
        m_UINTs.emplace_front((UINT)bAllow);
        m_Desc.back().pAllowSparseNodes = (BOOL*)&m_UINTs.front();
    }
    void MaxOutputRecords(UINT maxOutputRecords) noexcept
    {
        m_UINTs.emplace_front(maxOutputRecords);
        m_Desc.back().pMaxRecords = &m_UINTs.front();
    }
    void MaxOutputRecordsSharedWith(UINT outputIndex) noexcept
    {
        m_UINTs.emplace_front(outputIndex);
        m_Desc.back().pMaxRecordsSharedWithOutputIndex = &m_UINTs.front();
    }
private:
    std::vector<D3D12_NODE_OUTPUT_OVERRIDES> m_Desc;
    // Cached parameters
    CD3DX12_STATE_OBJECT_DESC::StringContainer<LPCWSTR, std::wstring> m_Strings;
    std::forward_list<UINT> m_UINTs;
    std::forward_list<D3D12_NODE_ID> m_NodeIDs;
    const D3D12_NODE_OUTPUT_OVERRIDES** m_ppOwner;
    UINT* m_pNumOutputOverrides;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_WORK_GRAPH_SUBOBJECT;

//------------------------------------------------------------------------------------------------
class CD3DX12_NODE_HELPER_BASE
{
protected:
    struct Backreference
    {
        CD3DX12_WORK_GRAPH_SUBOBJECT *m_pGraph;
        UINT m_NodeIndex;
    };
public:
    CD3DX12_NODE_HELPER_BASE(const Backreference &BackRef)
        : m_BackRef(BackRef)
    {
    }
    virtual ~CD3DX12_NODE_HELPER_BASE() = default;
protected:
    D3D12_NODE *GetNode() const;
    const Backreference m_BackRef;
    CD3DX12_STATE_OBJECT_DESC::StringContainer<LPCWSTR, std::wstring> m_Strings;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_SHADER_NODE // Not specifying launch mode.
                          // Don't need to distinguish if no parameter overriding is happening
    : public CD3DX12_NODE_HELPER_BASE
{
public:
    CD3DX12_SHADER_NODE(
        const Backreference &BackRef,
        LPCWSTR _Shader = nullptr)
            : CD3DX12_NODE_HELPER_BASE(BackRef)
    {
        GetNode()->NodeType = D3D12_NODE_TYPE_SHADER;
        Shader(_Shader);
    }
    void Shader(LPCWSTR _Shader)
    {
        GetNode()->Shader.Shader = m_Strings.LocalCopy(_Shader);
    }
    LPCWSTR GetShaderName() const { return GetNode()->Shader.Shader; }
};

#endif // D3D12_SDK_VERSION >= 612

#if defined(D3D12_PREVIEW_SDK_VERSION) && (D3D12_PREVIEW_SDK_VERSION >= 713)

//------------------------------------------------------------------------------------------------
// Use this class when defining a dispatch mesh launch node where configuration parameters
// need to be overridden.  If overrides are not needed, just use CD3DX12_PROGRAM_NODE
class CD3DX12_MESH_LAUNCH_NODE_OVERRIDES
    : public CD3DX12_NODE_HELPER_BASE
{
public:
    CD3DX12_MESH_LAUNCH_NODE_OVERRIDES(
        const Backreference &BackRef,
        LPCWSTR _Program = nullptr)
            : CD3DX12_NODE_HELPER_BASE(BackRef)
    {
        Overrides = {};
        D3D12_NODE *pNode = GetNode();
        pNode->NodeType = D3D12_NODE_TYPE_PROGRAM;
        pNode->Program.OverridesType = D3D12_PROGRAM_NODE_OVERRIDES_TYPE_MESH_LAUNCH;
        pNode->Program.pMeshLaunchOverrides = &Overrides;
        Program(_Program);
    }
    void Program(LPCWSTR _Program)
    {
        GetNode()->Program.Program = m_Strings.LocalCopy(_Program);
    }
    LPCWSTR GetProgramName() const { return GetNode()->Program.Program; }
    void LocalRootArgumentsTableIndex(UINT index)
    {
        m_UINTs.emplace_front(index);
        Overrides.pLocalRootArgumentsTableIndex = &m_UINTs.front();
    }
    void ProgramEntry(BOOL bIsProgramEntry)
    {
        m_UINTs.emplace_front(bIsProgramEntry);
        Overrides.pProgramEntry = (BOOL*)&m_UINTs.front();
    }
    void NewName(D3D12_NODE_ID NodeID)
    {
        m_NodeIDs.emplace_front(D3D12_NODE_ID{ m_Strings.LocalCopy(NodeID.Name),NodeID.ArrayIndex });
        Overrides.pNewName = &m_NodeIDs.front();
    }
    void ShareInputOf(D3D12_NODE_ID NodeID)
    {
        m_NodeIDs.emplace_front(D3D12_NODE_ID{ m_Strings.LocalCopy(NodeID.Name),NodeID.ArrayIndex });
        Overrides.pShareInputOf = &m_NodeIDs.front();
    }
    void DispatchGrid(UINT x, UINT y, UINT z)
    {
        m_UINT3s.emplace_front(UINT3{ x,y,z });
        Overrides.pDispatchGrid = (UINT*)&m_UINT3s.front();
    }
    void MaxDispatchGrid(UINT x, UINT y, UINT z)
    {
        m_UINT3s.emplace_front(UINT3{ x,y,z });
        Overrides.pMaxDispatchGrid = (UINT*)&m_UINT3s.front();
    }
    void MaxInputRecordsPerGraphEntryRecord(UINT recordCount, BOOL bSharedAcrossNodeArray)
    {
        m_UINT3s.emplace_front(UINT3{ recordCount,(UINT)bSharedAcrossNodeArray,0 }); // using uint3 even though only 2 values are used
        Overrides.pMaxInputRecordsPerGraphEntryRecord = (D3D12_MAX_NODE_INPUT_RECORDS_PER_GRAPH_ENTRY_RECORD*)&m_UINT3s.front();
    }
    D3D12_MESH_LAUNCH_OVERRIDES Overrides;
private:
    // Cached parameters
    std::forward_list<UINT> m_UINTs;
    struct UINT3
    {
        UINT x;
        UINT y;
        UINT z;
    };
    std::forward_list<UINT3> m_UINT3s;
    std::forward_list<D3D12_NODE_ID> m_NodeIDs;
};

//------------------------------------------------------------------------------------------------
// Use this class when defining a program node where configuration parameters
// need to be overridden for parameters that are common to all program node types.
// This option is a convenience if you don't want to determine what the program launch mode is
// and just want to override a setting that isn't specific to mode.
// If overrides are not needed, just use CD3DX12_PROGRAM_NODE
class CD3DX12_COMMON_PROGRAM_NODE_OVERRIDES
    : public CD3DX12_NODE_HELPER_BASE
{
public:
    CD3DX12_COMMON_PROGRAM_NODE_OVERRIDES(
        const Backreference &BackRef,
        LPCWSTR _Program = nullptr)
            : CD3DX12_NODE_HELPER_BASE(BackRef)
    {
        Overrides = {};
        D3D12_NODE *pNode = GetNode();
        pNode->NodeType = D3D12_NODE_TYPE_PROGRAM;
        pNode->Program.OverridesType = D3D12_PROGRAM_NODE_OVERRIDES_TYPE_COMMON_PROGRAM;
        pNode->Program.pCommonProgramNodeOverrides = &Overrides;
        Program(_Program);
    }
    void Program(LPCWSTR _Program)
    {
        GetNode()->Program.Program = m_Strings.LocalCopy(_Program);
    }
    LPCWSTR GetProgramName() const { return GetNode()->Program.Program; }
    void LocalRootArgumentsTableIndex(UINT index)
    {
        m_UINTs.emplace_front(index);
        Overrides.pLocalRootArgumentsTableIndex = &m_UINTs.front();
    }
    void ProgramEntry(BOOL bIsProgramEntry)
    {
        m_UINTs.emplace_front(bIsProgramEntry);
        Overrides.pProgramEntry = (BOOL*)&m_UINTs.front();
    }
    void NewName(D3D12_NODE_ID NodeID)
    {
        m_NodeIDs.emplace_front(D3D12_NODE_ID{ m_Strings.LocalCopy(NodeID.Name),NodeID.ArrayIndex });
        Overrides.pNewName = &m_NodeIDs.front();
    }
    void ShareInputOf(D3D12_NODE_ID NodeID)
    {
        m_NodeIDs.emplace_front(D3D12_NODE_ID{ m_Strings.LocalCopy(NodeID.Name),NodeID.ArrayIndex });
        Overrides.pShareInputOf = &m_NodeIDs.front();
    }
    D3D12_COMMON_PROGRAM_NODE_OVERRIDES Overrides;
private:
    // Cached parameters
    std::forward_list<UINT> m_UINTs;
    std::forward_list<D3D12_NODE_ID> m_NodeIDs;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_PROGRAM_NODE // Not specifying launch mode.
    // Don't need to distinguish if no parameter overriding is happening
    : public CD3DX12_NODE_HELPER_BASE
{
public:
    CD3DX12_PROGRAM_NODE(
        const Backreference &BackRef,
        LPCWSTR _Program = nullptr)
            : CD3DX12_NODE_HELPER_BASE(BackRef)
    {
        D3D12_NODE *pNode = GetNode();
        pNode->NodeType = D3D12_NODE_TYPE_PROGRAM;
        Program(_Program);
    }
    void Program(LPCWSTR _Program)
    {
        GetNode()->Program.Program = m_Strings.LocalCopy(_Program);
    }
    LPCWSTR GetProgramName() const { return GetNode()->Program.Program; }
};

#endif // D3D12_PREVIEW_SDK_VERSION >= 713

#if defined(D3D12_SDK_VERSION) && (D3D12_SDK_VERSION >= 612)

//------------------------------------------------------------------------------------------------
// Use this class when defining a broadcasting launch node where configuration parameters
// need to be overridden.  If overrides are not needed, just use CD3DX12_COMPUTE_SHADER_NODE
class CD3DX12_BROADCASTING_LAUNCH_NODE_OVERRIDES
    : public CD3DX12_NODE_HELPER_BASE
{
public:
    CD3DX12_BROADCASTING_LAUNCH_NODE_OVERRIDES(
        const Backreference &BackRef,
        LPCWSTR _Shader = nullptr) :
            CD3DX12_NODE_HELPER_BASE(BackRef),
            m_NodeOutputOverrides(&Overrides.pOutputOverrides, &Overrides.NumOutputOverrides)
    {
        Overrides = {};
        D3D12_NODE *pNode = GetNode();
        pNode->NodeType = D3D12_NODE_TYPE_SHADER;
        pNode->Shader.OverridesType = D3D12_NODE_OVERRIDES_TYPE_BROADCASTING_LAUNCH;
        pNode->Shader.pBroadcastingLaunchOverrides = &Overrides;
        Shader(_Shader);
    }
    void Shader(LPCWSTR _Shader)
    {
        GetNode()->Shader.Shader = m_Strings.LocalCopy(_Shader);
    }
    LPCWSTR GetShaderName() const { return GetNode()->Shader.Shader; }
    void LocalRootArgumentsTableIndex(UINT index)
    {
        m_UINTs.emplace_front(index);
        Overrides.pLocalRootArgumentsTableIndex = &m_UINTs.front();
    }
    void ProgramEntry(BOOL bIsProgramEntry)
    {
        m_UINTs.emplace_front(bIsProgramEntry);
        Overrides.pProgramEntry = (BOOL*)&m_UINTs.front();
    }
    void NewName(D3D12_NODE_ID NodeID)
    {
        m_NodeIDs.emplace_front(D3D12_NODE_ID{ m_Strings.LocalCopy(NodeID.Name),NodeID.ArrayIndex });
        Overrides.pNewName = &m_NodeIDs.front();
    }
    void ShareInputOf(D3D12_NODE_ID NodeID)
    {
        m_NodeIDs.emplace_front(D3D12_NODE_ID{ m_Strings.LocalCopy(NodeID.Name),NodeID.ArrayIndex });
        Overrides.pShareInputOf = &m_NodeIDs.front();
    }
    void DispatchGrid(UINT x, UINT y, UINT z)
    {
        m_UINT3s.emplace_front(UINT3{ x,y,z });
        Overrides.pDispatchGrid = (UINT*)&m_UINT3s.front();
    }
    void MaxDispatchGrid(UINT x, UINT y, UINT z)
    {
        m_UINT3s.emplace_front(UINT3{x,y,z});
        Overrides.pMaxDispatchGrid = (UINT*)&m_UINT3s.front();
    }
    CD3DX12_NODE_OUTPUT_OVERRIDES& NodeOutputOverrides()
    {
        return m_NodeOutputOverrides;
    }
    D3D12_BROADCASTING_LAUNCH_OVERRIDES Overrides;
private:
    // Cached parameters
    std::forward_list<UINT> m_UINTs;
    struct UINT3
    {
        UINT x;
        UINT y;
        UINT z;
    };
    std::forward_list<UINT3> m_UINT3s;
    std::forward_list<D3D12_NODE_ID> m_NodeIDs;
    CD3DX12_NODE_OUTPUT_OVERRIDES m_NodeOutputOverrides;
};

//------------------------------------------------------------------------------------------------
// Use this class when defining a coalescing launch node where configuration parameters
// need to be overridden.  If overrides are not needed, just use CD3DX12_COMPUTE_SHADER_NODE
class CD3DX12_COALESCING_LAUNCH_NODE_OVERRIDES
    : public CD3DX12_NODE_HELPER_BASE
{
public:
    CD3DX12_COALESCING_LAUNCH_NODE_OVERRIDES(
        const Backreference &BackRef,
        LPCWSTR _Shader = nullptr) :
            CD3DX12_NODE_HELPER_BASE(BackRef),
            m_NodeOutputOverrides(&Overrides.pOutputOverrides, &Overrides.NumOutputOverrides)
    {
        Overrides = {};
        D3D12_NODE *pNode = GetNode();
        pNode->NodeType = D3D12_NODE_TYPE_SHADER;
        pNode->Shader.OverridesType = D3D12_NODE_OVERRIDES_TYPE_COALESCING_LAUNCH;
        pNode->Shader.pCoalescingLaunchOverrides = &Overrides;
        Shader(_Shader);
    }
    void Shader(LPCWSTR _Shader)
    {
        GetNode()->Shader.Shader = m_Strings.LocalCopy(_Shader);
    }
    LPCWSTR GetShaderName() const { return GetNode()->Shader.Shader; }
    void LocalRootArgumentsTableIndex(UINT index)
    {
        m_UINTs.emplace_front(index);
        Overrides.pLocalRootArgumentsTableIndex = &m_UINTs.front();
    }
    void ProgramEntry(BOOL bIsProgramEntry)
    {
        m_UINTs.emplace_front(bIsProgramEntry);
        Overrides.pProgramEntry = (BOOL*)&m_UINTs.front();
    }
    void NewName(D3D12_NODE_ID NodeID)
    {
        m_NodeIDs.emplace_front(D3D12_NODE_ID{ m_Strings.LocalCopy(NodeID.Name),NodeID.ArrayIndex });
        Overrides.pNewName = &m_NodeIDs.front();
    }
    void ShareInputOf(D3D12_NODE_ID NodeID)
    {
        m_NodeIDs.emplace_front(D3D12_NODE_ID{ m_Strings.LocalCopy(NodeID.Name),NodeID.ArrayIndex });
        Overrides.pShareInputOf = &m_NodeIDs.front();
    }
    CD3DX12_NODE_OUTPUT_OVERRIDES& NodeOutputOverrides()
    {
        return m_NodeOutputOverrides;
    }
    D3D12_COALESCING_LAUNCH_OVERRIDES Overrides;
private:
    // Cached parameters
    std::forward_list<UINT> m_UINTs;
    struct UINT3
    {
        UINT x;
        UINT y;
        UINT z;
    };
    std::forward_list<UINT3> m_UINT3s;
    std::forward_list<D3D12_NODE_ID> m_NodeIDs;
    CD3DX12_NODE_OUTPUT_OVERRIDES m_NodeOutputOverrides;
};

//------------------------------------------------------------------------------------------------
// Use this class when defining a thread launch node where configuration parameters
// need to be overridden.  If overrides are not needed, just use CD3DX12_COMPUTE_SHADER_NODE
class CD3DX12_THREAD_LAUNCH_NODE_OVERRIDES
    : public CD3DX12_NODE_HELPER_BASE
{
public:
    CD3DX12_THREAD_LAUNCH_NODE_OVERRIDES(
        const Backreference &BackRef,
        LPCWSTR _Shader = nullptr) :
            CD3DX12_NODE_HELPER_BASE(BackRef),
            m_NodeOutputOverrides(&Overrides.pOutputOverrides, &Overrides.NumOutputOverrides)
    {
        Overrides = {};
        D3D12_NODE *pNode = GetNode();
        pNode->NodeType = D3D12_NODE_TYPE_SHADER;
        pNode->Shader.OverridesType = D3D12_NODE_OVERRIDES_TYPE_THREAD_LAUNCH;
        pNode->Shader.pThreadLaunchOverrides = &Overrides;
        Shader(_Shader);
    }
    void Shader(LPCWSTR _Shader)
    {
        GetNode()->Shader.Shader = m_Strings.LocalCopy(_Shader);
    }
    LPCWSTR GetShaderName() const { return GetNode()->Shader.Shader; }
    void LocalRootArgumentsTableIndex(UINT index)
    {
        m_UINTs.emplace_front(index);
        Overrides.pLocalRootArgumentsTableIndex = &m_UINTs.front();
    }
    void ProgramEntry(BOOL bIsProgramEntry)
    {
        m_UINTs.emplace_front(bIsProgramEntry);
        Overrides.pProgramEntry = (BOOL*)&m_UINTs.front();
    }
    void NewName(D3D12_NODE_ID NodeID)
    {
        m_NodeIDs.emplace_front(D3D12_NODE_ID{ m_Strings.LocalCopy(NodeID.Name),NodeID.ArrayIndex });
        Overrides.pNewName = &m_NodeIDs.front();
    }
    void ShareInputOf(D3D12_NODE_ID NodeID)
    {
        m_NodeIDs.emplace_front(D3D12_NODE_ID{ m_Strings.LocalCopy(NodeID.Name),NodeID.ArrayIndex });
        Overrides.pShareInputOf = &m_NodeIDs.front();
    }
    CD3DX12_NODE_OUTPUT_OVERRIDES& NodeOutputOverrides()
    {
        return m_NodeOutputOverrides;
    }
    D3D12_THREAD_LAUNCH_OVERRIDES Overrides;
private:
    // Cached parameters
    std::forward_list<UINT> m_UINTs;
    std::forward_list<D3D12_NODE_ID> m_NodeIDs;
    CD3DX12_NODE_OUTPUT_OVERRIDES m_NodeOutputOverrides;
};

//------------------------------------------------------------------------------------------------
// Use this class when defining a node where configuration parameters
// need to be overridden for parameters that are common to all launch node types.
// This option is a convenience if you don't want to determine what the launch mode is
// and just want to override a setting that isn't specific to launch mode.
// If overrides are not needed, just use CD3DX12_COMPUTE_SHADER_NODE
class CD3DX12_COMMON_COMPUTE_NODE_OVERRIDES
    : public CD3DX12_NODE_HELPER_BASE
{
public:
    CD3DX12_COMMON_COMPUTE_NODE_OVERRIDES(
        const Backreference &BackRef,
        LPCWSTR _Shader = nullptr) :
            CD3DX12_NODE_HELPER_BASE(BackRef),
            m_NodeOutputOverrides(&Overrides.pOutputOverrides, &Overrides.NumOutputOverrides)
    {
        Overrides = {};
        D3D12_NODE *pNode = GetNode();
        pNode->NodeType = D3D12_NODE_TYPE_SHADER;
        pNode->Shader.OverridesType = D3D12_NODE_OVERRIDES_TYPE_COMMON_COMPUTE;
        pNode->Shader.pThreadLaunchOverrides = &Overrides;
        Shader(_Shader);
    }
    void Shader(LPCWSTR _Shader)
    {
        GetNode()->Shader.Shader = m_Strings.LocalCopy(_Shader);
    }
    LPCWSTR GetShaderName() const { return GetNode()->Shader.Shader; }
    void LocalRootArgumentsTableIndex(UINT index)
    {
        m_UINTs.emplace_front(index);
        Overrides.pLocalRootArgumentsTableIndex = &m_UINTs.front();
    }
    void ProgramEntry(BOOL bIsProgramEntry)
    {
        m_UINTs.emplace_front(bIsProgramEntry);
        Overrides.pProgramEntry = (BOOL*)&m_UINTs.front();
    }
    void NewName(D3D12_NODE_ID NodeID)
    {
        m_NodeIDs.emplace_front(D3D12_NODE_ID{ m_Strings.LocalCopy(NodeID.Name),NodeID.ArrayIndex });
        Overrides.pNewName = &m_NodeIDs.front();
    }
    void ShareInputOf(D3D12_NODE_ID NodeID)
    {
        m_NodeIDs.emplace_front(D3D12_NODE_ID{ m_Strings.LocalCopy(NodeID.Name),NodeID.ArrayIndex });
        Overrides.pShareInputOf = &m_NodeIDs.front();
    }
    CD3DX12_NODE_OUTPUT_OVERRIDES& NodeOutputOverrides()
    {
        return m_NodeOutputOverrides;
    }
    D3D12_THREAD_LAUNCH_OVERRIDES Overrides;
private:
    // Cached parameters
    std::forward_list<UINT> m_UINTs;
    std::forward_list<D3D12_NODE_ID> m_NodeIDs;
    CD3DX12_NODE_OUTPUT_OVERRIDES m_NodeOutputOverrides;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_WORK_GRAPH_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_WORK_GRAPH_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_WORK_GRAPH_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_WORK_GRAPH;
    }
    void IncludeAllAvailableNodes()
    {
        m_Desc.Flags |= D3D12_WORK_GRAPH_FLAG_INCLUDE_ALL_AVAILABLE_NODES;
    }

    void EntrypointGraphicsNodesRasterizeInOrder()
    {
        m_Desc.Flags |= D3D12_WORK_GRAPH_FLAG_ENTRYPOINT_GRAPHICS_NODES_RASTERIZE_IN_ORDER;
    }

    void SetProgramName(LPCWSTR ProgramName)
    {
        m_Desc.ProgramName = m_Strings.LocalCopy(ProgramName);
    }
    void SetFlags(D3D12_WORK_GRAPH_FLAGS Flags)
    {
        m_Desc.Flags = Flags;
    }
    void AddEntrypoint(D3D12_NODE_ID Entrypoint)
    {
        m_Entrypoints.emplace_back(D3D12_NODE_ID{ m_Strings.LocalCopy(Entrypoint.Name),Entrypoint.ArrayIndex });
        m_Desc.NumEntrypoints++;
        m_Desc.pEntrypoints = m_Entrypoints.data();
    }
    // Shallow copy of explicitly defined nodes. Caller must keep pNodes alive.
    void SetExplicitlyDefinedNodes(const D3D12_NODE* pNodes, UINT numNodes)
    {
        m_Desc.NumExplicitlyDefinedNodes = numNodes;
        m_Desc.pExplicitlyDefinedNodes = pNodes;
    }

    template<typename T>
    T* CreateNode()
    {
        m_NodeDescs.push_back({});
        m_Desc.NumExplicitlyDefinedNodes++;
        m_Desc.pExplicitlyDefinedNodes = m_NodeDescs.data();
        T* pNodeHelper = new T({this, (UINT)m_NodeDescs.size() - 1});
        m_OwnedNodeHelpers.emplace_back(pNodeHelper);
        return pNodeHelper;
    }
    CD3DX12_SHADER_NODE* CreateShaderNode(LPCWSTR Shader = nullptr)
    {
        auto pNode = CreateNode<CD3DX12_SHADER_NODE>();
        pNode->Shader(Shader);
        return pNode;
    }
    CD3DX12_BROADCASTING_LAUNCH_NODE_OVERRIDES* CreateBroadcastingLaunchNodeOverrides(LPCWSTR Shader = nullptr)
    {
        auto pNode = CreateNode<CD3DX12_BROADCASTING_LAUNCH_NODE_OVERRIDES>();
        pNode->Shader(Shader);
        return pNode;
    }
    CD3DX12_COALESCING_LAUNCH_NODE_OVERRIDES* CreateCoalescingLaunchNodeOverrides(LPCWSTR Shader = nullptr)
    {
        auto pNode = CreateNode<CD3DX12_COALESCING_LAUNCH_NODE_OVERRIDES>();
        pNode->Shader(Shader);
        return pNode;
    }
    CD3DX12_THREAD_LAUNCH_NODE_OVERRIDES* CreateThreadLaunchNodeOverrides(LPCWSTR Shader = nullptr)
    {
        auto pNode = CreateNode<CD3DX12_THREAD_LAUNCH_NODE_OVERRIDES>();
        pNode->Shader(Shader);
        return pNode;
    }
    CD3DX12_COMMON_COMPUTE_NODE_OVERRIDES* CreateCommonComputeNodeOverrides(LPCWSTR Shader = nullptr)
    {
        auto pNode = CreateNode<CD3DX12_COMMON_COMPUTE_NODE_OVERRIDES>();
        pNode->Shader(Shader);
        return pNode;
    }
#endif // D3D12_SDK_VERSION >= 612

#if defined(D3D12_PREVIEW_SDK_VERSION) && (D3D12_PREVIEW_SDK_VERSION >= 713)
    CD3DX12_PROGRAM_NODE* CreateProgramNode(LPCWSTR Program = nullptr)
    {
        auto pNode = CreateNode<CD3DX12_PROGRAM_NODE>();
        pNode->Program(Program);
        return pNode;
    }
    CD3DX12_MESH_LAUNCH_NODE_OVERRIDES* CreateMeshLaunchNodeOverrides(LPCWSTR Program = nullptr)
    {
        auto pNode = CreateNode<CD3DX12_MESH_LAUNCH_NODE_OVERRIDES>();
        pNode->Program(Program);
        return pNode;
    }
    CD3DX12_COMMON_PROGRAM_NODE_OVERRIDES* CreateCommonProgramNodeOverrides(LPCWSTR Program = nullptr)
    {
        auto pNode = CreateNode<CD3DX12_COMMON_PROGRAM_NODE_OVERRIDES>();
        pNode->Program(Program);
        return pNode;
    }
#endif // D3D12_PREVIEW_SDK_VERSION >= 713

#if defined(D3D12_SDK_VERSION) && (D3D12_SDK_VERSION >= 612)

    operator const D3D12_WORK_GRAPH_DESC& () noexcept
    {
        return m_Desc;
    }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        m_Entrypoints.clear();
        m_NodeDescs.clear();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_WORK_GRAPH_DESC m_Desc;
    std::vector<D3D12_NODE_ID> m_Entrypoints;
    std::vector<D3D12_NODE> m_NodeDescs;
    CD3DX12_STATE_OBJECT_DESC::StringContainer<LPCWSTR, std::wstring> m_Strings;
    std::vector<std::unique_ptr<const CD3DX12_NODE_HELPER_BASE>> m_OwnedNodeHelpers;
    friend class CD3DX12_NODE_HELPER_BASE;
    friend class CD3DX12_STATE_OBJECT_DESC;
};

inline D3D12_NODE * CD3DX12_NODE_HELPER_BASE::GetNode() const
{
    return &m_BackRef.m_pGraph->m_NodeDescs[m_BackRef.m_NodeIndex];
}
#endif // D3D12_SDK_VERSION >= 612

//------------------------------------------------------------------------------------------------
class CD3DX12_PARTIAL_GRAPHICS_PROGRAM_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_PARTIAL_GRAPHICS_PROGRAM_SUBOBJECT()
    {
        Init();
    }
    CD3DX12_PARTIAL_GRAPHICS_PROGRAM_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetProgramName(LPCWSTR ProgramName)
    {
        m_Desc.ProgramName = m_Strings.LocalCopy(ProgramName);
    }
    void AddExport(LPCWSTR exportName)
    {
        m_Exports.emplace_back(m_Strings.LocalCopy(exportName));
        m_Desc.NumExports++;
        // Below: using ugly way to get pointer in case .data() is not defined
        m_Desc.pExports = &m_Exports[0];
    }
    void AddSubobject(const D3D12_STATE_SUBOBJECT& subobject)
    {
        m_Subobjects.emplace_back(&subobject);
        m_Desc.NumSubobjects++;
        // Below: using ugly way to get pointer in case .data() is not defined
        m_Desc.ppSubobjects = &m_Subobjects[0];
    }
    void SetPartialGraphicsProgramType(D3D12_PARTIAL_GRAPHICS_PROGRAM_TYPE type)
    {
        m_Desc.ProgramType = type;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_PARTIAL_GRAPHICS_PROGRAM;
    }
    operator const D3D12_PARTIAL_GRAPHICS_PROGRAM_DESC& () const noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        m_Subobjects.clear();
        m_Strings.clear();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_PARTIAL_GRAPHICS_PROGRAM_DESC m_Desc;
    std::vector<LPCWSTR> m_Exports;
    std::vector<D3D12_STATE_SUBOBJECT const*> m_Subobjects;
    CD3DX12_STATE_OBJECT_DESC::StringContainer<LPCWSTR, std::wstring> m_Strings;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_OUTPUT_LINKAGE_SIGNATURE_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_OUTPUT_LINKAGE_SIGNATURE_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_OUTPUT_LINKAGE_SIGNATURE_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void AddOutputLinkageElementDesc(D3D12_OUTPUT_LINKAGE_ELEMENT_DESC outputLinkageElementDesc)
    {
        m_OutputLinkageElements.emplace_back(
            D3D12_OUTPUT_LINKAGE_ELEMENT_DESC{
                m_Strings.LocalCopy(outputLinkageElementDesc.SemanticName),
                outputLinkageElementDesc.SemanticIndex,
                outputLinkageElementDesc.StartComponent,
                outputLinkageElementDesc.ComponentCount
            });
        ++m_Desc.NumElements;
        // Below: using ugly way to get pointer in case .data() is not defined
        m_Desc.pOutputLinkageElementDescs = &m_OutputLinkageElements[0];
    }
    // Populate from PS shader reflection, extracting the input parameters.
    // The output linkage describes what the PS input the prerast stage should expect from the PS stage.
    HRESULT PopulateFromReflection(ID3D12ShaderReflection* pReflection)
    {
        if (!pReflection) return E_INVALIDARG;

        D3D12_SHADER_DESC shaderDesc = {};
        HRESULT hr = pReflection->GetDesc(&shaderDesc);
        if (FAILED(hr)) return hr;

        for (UINT i = 0; i < shaderDesc.InputParameters; ++i)
        {
            D3D12_SIGNATURE_PARAMETER_DESC paramDesc = {};
            hr = pReflection->GetInputParameterDesc(i, &paramDesc);
            if (FAILED(hr)) return hr;

             // For input signatures: ReadWriteMask == components always read
             if (paramDesc.ReadWriteMask == 0)
             {
                 continue; // shader never reads this input, skip it
             }

            BYTE mask = paramDesc.Mask;
            BYTE startComponent = 0;
            while (startComponent < 4 && !(mask & (1 << startComponent)))
                startComponent++;

            // Component count will include gaps
            BYTE componentCount = 0;
            for (BYTE c = startComponent; c < 4; ++c)
            {
                if (mask & (1 << c))
                    componentCount++;
            }

            AddOutputLinkageElementDesc({
                paramDesc.SemanticName,
                paramDesc.SemanticIndex,
                startComponent,
                componentCount
            });
        }

        return S_OK;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type()const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_OUTPUT_LINKAGE_SIGNATURE;
    }
    operator const D3D12_OUTPUT_LINKAGE_SIGNATURE_DESC&() const noexcept { return m_Desc; }
    operator D3D12_OUTPUT_LINKAGE_SIGNATURE_DESC&() noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        m_OutputLinkageElements.clear();
        m_Strings.clear();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_OUTPUT_LINKAGE_SIGNATURE_DESC m_Desc;
    std::vector<D3D12_OUTPUT_LINKAGE_ELEMENT_DESC> m_OutputLinkageElements;
    CD3DX12_STATE_OBJECT_DESC::StringContainer<LPCSTR, std::string> m_Strings;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_PRERASTERIZATION_OUTPUT_LINKAGE_SIGNATURE_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_PRERASTERIZATION_OUTPUT_LINKAGE_SIGNATURE_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_PRERASTERIZATION_OUTPUT_LINKAGE_SIGNATURE_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void AddPrerasterizationOutputLinkageElementDesc(D3D12_PRERASTERIZATION_OUTPUT_LINKAGE_ELEMENT_DESC prerasterizationOutputLinkageElementDesc)
    {
        m_PrerasterizationOutputLinkageElements.emplace_back(
            D3D12_PRERASTERIZATION_OUTPUT_LINKAGE_ELEMENT_DESC{
                m_Strings.LocalCopy(prerasterizationOutputLinkageElementDesc.SemanticName),
                prerasterizationOutputLinkageElementDesc.SemanticIndex,
                prerasterizationOutputLinkageElementDesc.StartComponent,
                prerasterizationOutputLinkageElementDesc.ComponentCount
            });
        ++m_Desc.NumElements;
        // Below: using ugly way to get pointer in case .data() is not defined
        m_Desc.pOutputLinkageElementDescs = &m_PrerasterizationOutputLinkageElements[0];
    }
    // Populate from shader reflection, extracting all the output parameters.
    // The prerast output linkage describes the prerast output signature the PS stage should expect.
    HRESULT PopulateFromReflection(ID3D12ShaderReflection* pReflection)
    {
        if (!pReflection) return E_INVALIDARG;

        D3D12_SHADER_DESC shaderDesc = {};
        HRESULT hr = pReflection->GetDesc(&shaderDesc);
        if (FAILED(hr)) return hr;

        for (UINT i = 0; i < shaderDesc.OutputParameters; ++i)
        {
            D3D12_SIGNATURE_PARAMETER_DESC paramDesc = {};
            hr = pReflection->GetOutputParameterDesc(i, &paramDesc);
            if (FAILED(hr)) return hr;

            BYTE mask = paramDesc.Mask;
            BYTE startComponent = 0;
            while (startComponent < 4 && !(mask & (1 << startComponent)))
                startComponent++;

            // component count will include gaps
            BYTE componentCount = 0;
            for (BYTE c = startComponent; c < 4; ++c)
            {
                if (mask & (1 << c))
                    componentCount++;
            }

            AddPrerasterizationOutputLinkageElementDesc({
                paramDesc.SemanticName,
                paramDesc.SemanticIndex,
                startComponent,
                componentCount
            });
        }

        return S_OK;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_PRERASTERIZATION_OUTPUT_LINKAGE_SIGNATURE;
    }
    operator const D3D12_PRERASTERIZATION_OUTPUT_LINKAGE_SIGNATURE_DESC&() const noexcept { return m_Desc; }
    operator D3D12_PRERASTERIZATION_OUTPUT_LINKAGE_SIGNATURE_DESC&() noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        m_PrerasterizationOutputLinkageElements.clear();
        m_Strings.clear();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_PRERASTERIZATION_OUTPUT_LINKAGE_SIGNATURE_DESC m_Desc;
    std::vector<D3D12_PRERASTERIZATION_OUTPUT_LINKAGE_ELEMENT_DESC> m_PrerasterizationOutputLinkageElements;
    CD3DX12_STATE_OBJECT_DESC::StringContainer<LPCSTR, std::string> m_Strings;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_PRERASTERIZATION_SHADERS_PARTIAL_PROGRAM_FIELDS_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_PRERASTERIZATION_SHADERS_PARTIAL_PROGRAM_FIELDS_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_PRERASTERIZATION_SHADERS_PARTIAL_PROGRAM_FIELDS_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }

    void SetExcludePS(BOOL excludePS)
    {
        m_Desc.ExcludePS = excludePS;
    }

    void SetLateLinkInputLayoutSubobject(BOOL lateLinkInputLayoutSubobject)
    {
        m_Desc.LateLinkInputLayoutSubobject = lateLinkInputLayoutSubobject;
    }

    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_PRERASTERIZATION_SHADERS_PARTIAL_PROGRAM_FIELDS;
    }
    operator const D3D12_PRERASTERIZATION_SHADERS_PARTIAL_PROGRAM_FIELDS_DESC&() const noexcept { return m_Desc; }
    operator D3D12_PRERASTERIZATION_SHADERS_PARTIAL_PROGRAM_FIELDS_DESC&() noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_PRERASTERIZATION_SHADERS_PARTIAL_PROGRAM_FIELDS_DESC m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_PIXEL_SHADER_PARTIAL_PROGRAM_FIELDS_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_PIXEL_SHADER_PARTIAL_PROGRAM_FIELDS_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_PIXEL_SHADER_PARTIAL_PROGRAM_FIELDS_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetLineRasterizationMode(D3D12_LINE_RASTERIZATION_MODE mode)
    {
        m_Desc.LineRasterizationMode = mode;
    }
    void SetForcedSampleCount(UINT sampleCount)
    {
        m_Desc.ForcedSampleCount = sampleCount;
    }
    void SetAlphaToCoverageEnable(BOOL enable)
    {
        m_Desc.AlphaToCoverageEnable = enable;
    }
    void SetDualSourceBlendEnable(BOOL enable)
    {
        m_Desc.DualSourceBlendEnable = enable;
    }
    void SetLateLinkRasterizerSubobject(BOOL lateLinkRasterizerSubobject)
    {
        m_Desc.LateLinkRasterizerSubobject = lateLinkRasterizerSubobject;
    }
    void SetLateLinkBlendSubobject(BOOL lateLinkBlendSubobject)
    {
        m_Desc.LateLinkBlendSubobject = lateLinkBlendSubobject;
    }
    void SetLateLinkSampleMaskSubobject(BOOL lateLinkSampleMaskSubobject)
    {
        m_Desc.LateLinkSampleMaskSubobject = lateLinkSampleMaskSubobject;
    }
    void SetLateLinkSampleDescSubobject(BOOL lateLinkSampleDescSubobject)
    {
        m_Desc.LateLinkSampleDescSubobject = lateLinkSampleDescSubobject;
    }
    void SetLateLinkDepthStencilFormatSubobject(BOOL lateLinkDepthStencilFormatSubobject)
    {
        m_Desc.LateLinkDepthStencilFormatSubobject = lateLinkDepthStencilFormatSubobject;
    }
    void SetLateLinkRenderTargetFormatSubobject(BOOL lateLinkRenderTargetFormatSubobject)
    {
        m_Desc.LateLinkRenderTargetFormatSubobject = lateLinkRenderTargetFormatSubobject;
    }
    void SetLateLinkDepthStencilSubobject(BOOL lateLinkDepthStencilSubobject)
    {
        m_Desc.LateLinkDepthStencilSubobject = lateLinkDepthStencilSubobject;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_PIXEL_SHADER_PARTIAL_PROGRAM_FIELDS;
    }
    operator const D3D12_PIXEL_SHADER_PARTIAL_PROGRAM_FIELDS_DESC&() const noexcept { return m_Desc; }
    operator D3D12_PIXEL_SHADER_PARTIAL_PROGRAM_FIELDS_DESC&() noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_PIXEL_SHADER_PARTIAL_PROGRAM_FIELDS_DESC m_Desc;
};


//------------------------------------------------------------------------------------------------
// Deep-copies a D3D12_STATE_OBJECT_DESC into this builder. Strings and exports are
// deep-copied and owned by the builder. DXIL library bytecode blobs and work graph
// explicitly-defined node arrays are shallow-copied; callers must keep them alive.
inline HRESULT CD3DX12_STATE_OBJECT_DESC::InitFromDesc(const D3D12_STATE_OBJECT_DESC& desc)
{
    Init(desc.Type);

    if (!desc.pSubobjects || desc.NumSubobjects == 0)
    {
        return S_OK;
    }

    // Map from original subobject pointers to the new SUBOBJECT_HELPER_BASE pointers,
    // so we can fix up cross-references (associations, generic programs, etc.).
    std::unordered_map<const D3D12_STATE_SUBOBJECT*, SUBOBJECT_HELPER_BASE*> subobjectMap;

    // First pass: create all subobjects and populate their data.
    for (UINT i = 0; i < desc.NumSubobjects; i++)
    {
        const D3D12_STATE_SUBOBJECT& src = desc.pSubobjects[i];
        switch (src.Type)
        {
        case D3D12_STATE_SUBOBJECT_TYPE_STATE_OBJECT_CONFIG:
        {
            auto pSrc = static_cast<const D3D12_STATE_OBJECT_CONFIG*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT>();
            pDst->SetFlags(pSrc->Flags);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE:
        {
            auto pRootSig = *static_cast<ID3D12RootSignature* const*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
            pDst->SetRootSignature(pRootSig);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE:
        {
            auto pRootSig = *static_cast<ID3D12RootSignature* const*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
            pDst->SetRootSignature(pRootSig);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK:
        {
            auto pSrc = static_cast<const D3D12_NODE_MASK*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_NODE_MASK_SUBOBJECT>();
            pDst->SetNodeMask(pSrc->NodeMask);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY:
        {
            auto pSrc = static_cast<const D3D12_DXIL_LIBRARY_DESC*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
            pDst->SetDXILLibrary(&pSrc->DXILLibrary);
            for (UINT e = 0; e < pSrc->NumExports; e++)
            {
                pDst->DefineExport(
                    pSrc->pExports[e].Name,
                    pSrc->pExports[e].ExportToRename,
                    pSrc->pExports[e].Flags);
            }
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION:
        {
            auto pSrc = static_cast<const D3D12_EXISTING_COLLECTION_DESC*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_EXISTING_COLLECTION_SUBOBJECT>();
            pDst->SetExistingCollection(pSrc->pExistingCollection);
            for (UINT e = 0; e < pSrc->NumExports; e++)
            {
                pDst->DefineExport(
                    pSrc->pExports[e].Name,
                    pSrc->pExports[e].ExportToRename,
                    pSrc->pExports[e].Flags);
            }
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
        {
            // Subobject pointer fixup is deferred to second pass.
            auto pSrc = static_cast<const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
            for (UINT e = 0; e < pSrc->NumExports; e++)
            {
                pDst->AddExport(pSrc->pExports[e]);
            }
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
        {
            auto pSrc = static_cast<const D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION>();
            pDst->SetSubobjectNameToAssociate(pSrc->SubobjectToAssociate);
            for (UINT e = 0; e < pSrc->NumExports; e++)
            {
                pDst->AddExport(pSrc->pExports[e]);
            }
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG:
        {
            auto pSrc = static_cast<const D3D12_RAYTRACING_SHADER_CONFIG*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
            pDst->Config(pSrc->MaxPayloadSizeInBytes, pSrc->MaxAttributeSizeInBytes);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG:
        {
            auto pSrc = static_cast<const D3D12_RAYTRACING_PIPELINE_CONFIG*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
            pDst->Config(pSrc->MaxTraceRecursionDepth);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG1:
        {
            auto pSrc = static_cast<const D3D12_RAYTRACING_PIPELINE_CONFIG1*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT>();
            pDst->Config(pSrc->MaxTraceRecursionDepth, pSrc->Flags);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP:
        {
            auto pSrc = static_cast<const D3D12_HIT_GROUP_DESC*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
            pDst->SetHitGroupExport(pSrc->HitGroupExport);
            pDst->SetHitGroupType(pSrc->Type);
            pDst->SetAnyHitShaderImport(pSrc->AnyHitShaderImport);
            pDst->SetClosestHitShaderImport(pSrc->ClosestHitShaderImport);
            pDst->SetIntersectionShaderImport(pSrc->IntersectionShaderImport);
            subobjectMap[&src] = pDst;
            break;
        }
#if defined(D3D12_SDK_VERSION) && (D3D12_SDK_VERSION >= 612)
        case D3D12_STATE_SUBOBJECT_TYPE_GENERIC_PROGRAM:
        {
            // Subobject pointer fixup is deferred to second pass.
            auto pSrc = static_cast<const D3D12_GENERIC_PROGRAM_DESC*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_GENERIC_PROGRAM_SUBOBJECT>();
            pDst->SetProgramName(pSrc->ProgramName);
            for (UINT e = 0; e < pSrc->NumExports; e++)
            {
                pDst->AddExport(pSrc->pExports[e]);
            }
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_WORK_GRAPH:
        {
            auto pSrc = static_cast<const D3D12_WORK_GRAPH_DESC*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_WORK_GRAPH_SUBOBJECT>();
            pDst->SetProgramName(pSrc->ProgramName);
            pDst->SetFlags(pSrc->Flags);
            for (UINT e = 0; e < pSrc->NumEntrypoints; e++)
            {
                pDst->AddEntrypoint(pSrc->pEntrypoints[e]);
            }
            pDst->SetExplicitlyDefinedNodes(pSrc->pExplicitlyDefinedNodes, pSrc->NumExplicitlyDefinedNodes);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_STREAM_OUTPUT:
        {
            auto pSrc = static_cast<const D3D12_STREAM_OUTPUT_DESC*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_STREAM_OUTPUT_SUBOBJECT>();
            pDst->SetSODeclEntries(pSrc->pSODeclaration, pSrc->NumEntries);
            pDst->SetBufferStrides(pSrc->pBufferStrides, pSrc->NumStrides);
            pDst->SetRasterizedStream(pSrc->RasterizedStream);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_BLEND:
        {
            auto pSrc = static_cast<const D3D12_BLEND_DESC*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_BLEND_SUBOBJECT>();
            pDst->SetAlphaToCoverageEnable(pSrc->AlphaToCoverageEnable);
            pDst->SetIndependentBlendEnable(pSrc->IndependentBlendEnable);
            for (UINT rt = 0; rt < 8; rt++)
            {
                pDst->SetRenderTarget(rt, pSrc->RenderTarget[rt]);
            }
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_RASTERIZER:
        {
            auto pSrc = static_cast<const D3D12_RASTERIZER_DESC2*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_RASTERIZER_SUBOBJECT>(*pSrc);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL2:
        {
            auto pSrc = static_cast<const D3D12_DEPTH_STENCIL_DESC2*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_DEPTH_STENCIL2_SUBOBJECT>(*pSrc);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT:
        {
            auto pSrc = static_cast<const D3D12_INPUT_LAYOUT_DESC*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_INPUT_LAYOUT_SUBOBJECT>();
            for (UINT e = 0; e < pSrc->NumElements; e++)
            {
                pDst->AddInputLayoutElementDesc(pSrc->pInputElementDescs[e]);
            }
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_IB_STRIP_CUT_VALUE:
        {
            auto pSrc = static_cast<const D3D12_INDEX_BUFFER_STRIP_CUT_VALUE*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_IB_STRIP_CUT_VALUE_SUBOBJECT>(*pSrc);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY:
        {
            auto pSrc = static_cast<const D3D12_PRIMITIVE_TOPOLOGY_TYPE*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_PRIMITIVE_TOPOLOGY_SUBOBJECT>(*pSrc);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS:
        {
            auto pSrc = static_cast<const D3D12_RT_FORMAT_ARRAY*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_RENDER_TARGET_FORMATS_SUBOBJECT>(*pSrc);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT:
        {
            auto pSrc = static_cast<const DXGI_FORMAT*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_DEPTH_STENCIL_FORMAT_SUBOBJECT>(*pSrc);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_SAMPLE_DESC:
        {
            auto pSrc = static_cast<const DXGI_SAMPLE_DESC*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_SAMPLE_DESC_SUBOBJECT>(*pSrc);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_FLAGS:
        {
            auto pSrc = static_cast<const D3D12_PIPELINE_STATE_FLAGS*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_FLAGS_SUBOBJECT>(*pSrc);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_VIEW_INSTANCING:
        {
            auto pSrc = static_cast<const D3D12_VIEW_INSTANCING_DESC*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_VIEW_INSTANCING_SUBOBJECT>();
            for (UINT e = 0; e < pSrc->ViewInstanceCount; e++)
            {
                pDst->AddViewInstanceLocation(pSrc->pViewInstanceLocations[e]);
            }
            pDst->SetFlags(pSrc->Flags);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL:
        {
            auto pSrc = static_cast<const D3D12_DEPTH_STENCIL_DESC*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_DEPTH_STENCIL_SUBOBJECT>(*pSrc);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL1:
        {
            auto pSrc = static_cast<const D3D12_DEPTH_STENCIL_DESC1*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_DEPTH_STENCIL1_SUBOBJECT>(*pSrc);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_SAMPLE_MASK:
        {
            auto pSrc = static_cast<const UINT*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_SAMPLE_MASK_SUBOBJECT>(*pSrc);
            subobjectMap[&src] = pDst;
            break;
        }
#endif // D3D12_SDK_VERSION >= 612
#if defined(D3D12_SDK_VERSION) && (D3D12_SDK_VERSION >= 618)
        case D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_SERIALIZED_ROOT_SIGNATURE:
        {
            auto pSrc = static_cast<const D3D12_GLOBAL_SERIALIZED_ROOT_SIGNATURE*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_GLOBAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT>();
            pDst->SetRootSignature(&pSrc->Desc);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_LOCAL_SERIALIZED_ROOT_SIGNATURE:
        {
            auto pSrc = static_cast<const D3D12_LOCAL_SERIALIZED_ROOT_SIGNATURE*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_LOCAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT>();
            pDst->SetRootSignature(&pSrc->Desc);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION_BY_KEY:
        {
            auto pSrc = static_cast<const D3D12_EXISTING_COLLECTION_BY_KEY_DESC*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_EXISTING_COLLECTION_BY_KEY_SUBOBJECT>();
            pDst->SetExistingCollection(pSrc->pKey, pSrc->KeySize);
            for (UINT e = 0; e < pSrc->NumExports; e++)
            {
                pDst->DefineExport(
                    pSrc->pExports[e].Name,
                    pSrc->pExports[e].ExportToRename,
                    pSrc->pExports[e].Flags);
            }
            subobjectMap[&src] = pDst;
            break;
        }
#endif // D3D12_SDK_VERSION >= 618
        case D3D12_STATE_SUBOBJECT_TYPE_API_EXTENSION:
        {
            auto pSrc = static_cast<const D3D12_API_EXTENSION_DESC*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_API_EXTENSION_SUBOBJECT>();
            pDst->SetApiExtension(pSrc->pExtension);
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_PARTIAL_GRAPHICS_PROGRAM:
        {
            auto pSrc = static_cast<const D3D12_PARTIAL_GRAPHICS_PROGRAM_DESC*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_PARTIAL_GRAPHICS_PROGRAM_SUBOBJECT>();
            pDst->SetProgramName(pSrc->ProgramName);
            pDst->SetPartialGraphicsProgramType(pSrc->ProgramType);
            for (UINT e = 0; e < pSrc->NumExports; e++)
            {
                pDst->AddExport(pSrc->pExports[e]);
            }
            // Subobject pointer fixup deferred to second pass.
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_OUTPUT_LINKAGE_SIGNATURE:
        {
            auto pSrc = static_cast<const D3D12_OUTPUT_LINKAGE_SIGNATURE_DESC*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_OUTPUT_LINKAGE_SIGNATURE_SUBOBJECT>();
            for (UINT e = 0; e < pSrc->NumElements; e++)
            {
                pDst->AddOutputLinkageElementDesc(pSrc->pOutputLinkageElementDescs[e]);
            }
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_PRERASTERIZATION_OUTPUT_LINKAGE_SIGNATURE:
        {
            auto pSrc = static_cast<const D3D12_PRERASTERIZATION_OUTPUT_LINKAGE_SIGNATURE_DESC*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_PRERASTERIZATION_OUTPUT_LINKAGE_SIGNATURE_SUBOBJECT>();
            for (UINT e = 0; e < pSrc->NumElements; e++)
            {
                pDst->AddPrerasterizationOutputLinkageElementDesc(pSrc->pOutputLinkageElementDescs[e]);
            }
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_PRERASTERIZATION_SHADERS_PARTIAL_PROGRAM_FIELDS:
        {
            auto pSrc = static_cast<const D3D12_PRERASTERIZATION_SHADERS_PARTIAL_PROGRAM_FIELDS_DESC*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_PRERASTERIZATION_SHADERS_PARTIAL_PROGRAM_FIELDS_SUBOBJECT>();
            (D3D12_PRERASTERIZATION_SHADERS_PARTIAL_PROGRAM_FIELDS_DESC&)*pDst = *pSrc;
            subobjectMap[&src] = pDst;
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_PIXEL_SHADER_PARTIAL_PROGRAM_FIELDS:
        {
            auto pSrc = static_cast<const D3D12_PIXEL_SHADER_PARTIAL_PROGRAM_FIELDS_DESC*>(src.pDesc);
            auto pDst = CreateSubobject<CD3DX12_PIXEL_SHADER_PARTIAL_PROGRAM_FIELDS_SUBOBJECT>();
            (D3D12_PIXEL_SHADER_PARTIAL_PROGRAM_FIELDS_DESC&)*pDst = *pSrc;
            subobjectMap[&src] = pDst;
            break;
        }
        default:
            // Unknown subobject type
            return E_UNEXPECTED;
        }
    }

    // Second pass: fix up subobject pointer references for types that
    // reference other subobjects by pointer.
    for (UINT i = 0; i < desc.NumSubobjects; i++)
    {
        const D3D12_STATE_SUBOBJECT& src = desc.pSubobjects[i];
        auto it = subobjectMap.find(&src);
        if (it == subobjectMap.end())
        {
            continue;
        }

        if (src.Type == D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION)
        {
            auto pSrc = static_cast<const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(src.pDesc);
            auto pDst = static_cast<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT*>(it->second);
            if (pSrc->pSubobjectToAssociate)
            {
                auto targetIt = subobjectMap.find(pSrc->pSubobjectToAssociate);
                if (targetIt != subobjectMap.end())
                {
                    pDst->SetSubobjectToAssociate(*targetIt->second);
                }
            }
        }
#if defined(D3D12_SDK_VERSION) && (D3D12_SDK_VERSION >= 612)
        else if (src.Type == D3D12_STATE_SUBOBJECT_TYPE_GENERIC_PROGRAM)
        {
            auto pSrc = static_cast<const D3D12_GENERIC_PROGRAM_DESC*>(src.pDesc);
            auto pDst = static_cast<CD3DX12_GENERIC_PROGRAM_SUBOBJECT*>(it->second);
            for (UINT s = 0; s < pSrc->NumSubobjects; s++)
            {
                auto targetIt = subobjectMap.find(pSrc->ppSubobjects[s]);
                if (targetIt != subobjectMap.end())
                {
                    pDst->AddSubobject(*targetIt->second);
                }
            }
        }
#endif // D3D12_SDK_VERSION >= 612
        else if (src.Type == D3D12_STATE_SUBOBJECT_TYPE_PARTIAL_GRAPHICS_PROGRAM)
        {
            auto pSrc = static_cast<const D3D12_PARTIAL_GRAPHICS_PROGRAM_DESC*>(src.pDesc);
            auto pDst = static_cast<CD3DX12_PARTIAL_GRAPHICS_PROGRAM_SUBOBJECT*>(it->second);
            for (UINT s = 0; s < pSrc->NumSubobjects; s++)
            {
                auto targetIt = subobjectMap.find(pSrc->ppSubobjects[s]);
                if (targetIt != subobjectMap.end())
                {
                    pDst->AddSubobject(*targetIt->second);
                }
            }
        }
    }
    return S_OK;
}


#undef D3DX12_COM_PTR
#undef D3DX12_COM_PTR_GET
#undef D3DX12_COM_PTR_ADDRESSOF
