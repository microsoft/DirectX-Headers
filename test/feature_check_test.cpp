// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef _WIN32
#include <wsl/winadapter.h>
#endif

#include <directx/d3d12.h>
#include <directx/dxcore.h>
#include <directx/d3dx12.h>
#include "dxguids/dxguids.h"

int main()
{
    ID3D12Device *device = nullptr;

    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))) 
    {
        return -1;
    }

    CD3DX12FeatureSupport features;
    if (FAILED(features.Init(device)))
    {
        return -1;
    }

    // 0: D3D12_OPTIONS
    BOOL DoublePrecisionFloatShaderOps = features.DoublePrecisionFloatShaderOps();
    D3D12_CONSERVATIVE_RASTERIZATION_TIER ConservativeRasterizationTier = features.ConservativeRasterizationTier();

    // 2: Feature Levels
    D3D_FEATURE_LEVEL HighestLevelSupported = features.HighestFeatureLevel();

    // 3: Format Support
    D3D12_FORMAT_SUPPORT1 Support1;
    D3D12_FORMAT_SUPPORT2 Support2;
    if (FAILED(features.FormatSupport(DXGI_FORMAT_R16G16B16A16_TYPELESS, Support1, Support2))) {
        return -1;
    }

    // 4: Multisample Quality Support
    UINT NumQualityLevels;
    if (FAILED(features.MultisampleQualityLevels(DXGI_FORMAT_R16G16B16A16_TYPELESS, 1, D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE, NumQualityLevels))) {
        return -1;
    }

    // 5: Format Info
    UINT8 PlaneCount;
    if (FAILED(features.FormatInfo(DXGI_FORMAT_R16G16B16A16_TYPELESS, PlaneCount))) {
        return -1;
    }

    // 6: GPU Virtual Address Support
    UINT MaxGPUVABitsPerProcess = features.MaxGPUVirtualAddressBitsPerProcess();
    UINT MaxGPUVABitsPerResource = features.MaxGPUVirtualAddressBitsPerResource();

    // 7: Shader Model
    D3D_SHADER_MODEL HighestShaderModel = features.HighestShaderModel();
    
    // 8: Options1
    BOOL WaveOps = features.WaveOps();
    UINT WaveLaneCountMin = features.WaveLaneCountMin();
    UINT WaveLaneCountMax = features.WaveLaneCountMax();

    // 10: Protected Resource Session Support
    D3D12_PROTECTED_RESOURCE_SESSION_SUPPORT_FLAGS ProtectedResourceSessionSupport = features.ProtectedResourceSessionSupport();

    // 12: Root Signature
    D3D_ROOT_SIGNATURE_VERSION HighestRootSignatureVersion = features.HighestRootSignatureVersion();

    // 16: Architecture1
    BOOL IsolatedMMU = features.IsolatedMMU();
    BOOL TileBasedRenderer = features.TileBasedRenderer();
    BOOL UMA = features.UMA();
    BOOL CacheCoherentUMA = features.CacheCoherentUMA();

    // 18: Options2
    BOOL DepthBoundsTestSupported = features.DepthBoundsTestSupported();
    D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER ProgrammableSamplePositionsTier = features.ProgrammableSamplePositionsTier();

    // 19: Shader Cache
    D3D12_SHADER_CACHE_SUPPORT_FLAGS ShaderCacheSupportFlags = features.ShaderCacheSupportFlags();

    // 20: Command Queue Prioirity
    BOOL CommandQueuePrioritySupportedPositive = features.CommandQueuePrioritySupported(D3D12_COMMAND_LIST_TYPE_DIRECT, 0);
    BOOL CommandQueuePrioritySupportedNegative = features.CommandQueuePrioritySupported(D3D12_COMMAND_LIST_TYPE_COPY, D3D12_COMMAND_QUEUE_PRIORITY_GLOBAL_REALTIME);
    BOOL CommandQueuePrioritySupportedInvalid = features.CommandQueuePrioritySupported((D3D12_COMMAND_LIST_TYPE)7, 0);
    
    // 21: Options3
    BOOL CopyQueueTimestampQueriesSupported = features.CopyQueueTimestampQueriesSupported();
    BOOL CastingFullyTypedFormatSupported = features.CastingFullyTypedFormatSupported();
    D3D12_COMMAND_LIST_SUPPORT_FLAGS WriteBufferImmediateSupportFlags = features.WriteBufferImmediateSupportFlags();
    D3D12_VIEW_INSTANCING_TIER ViewInstancingTier = features.ViewInstancingTier();
    BOOL BarycentricsSupported = features.BarycentricsSupported();

    // 22: Existing Heaps
    BOOL ExistingHeapsSupported = features.ExistingHeapsSupported();

    // 23: D3D12 Options4
    BOOL MSAA64KBAlgnedTextureSupported = features.MSAA64KBAlignedTextureSupported();
    D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER SharedResourceCompatibilityTier = features.SharedResourceCompatibilityTier();
    BOOL Native16BitShaderOpsSupported = features.Native16BitShaderOpsSupported();

    // 24: Serialization
    D3D12_HEAP_SERIALIZATION_TIER HeapSerializationTier = features.HeapSerializationTier();

    // 25: Cross Node
    //  D3D12_CROSS_NODE_SHARING_TIER CrossNodeSharingTier = features.CrossNodeSharingTier(); // Same option in D3D12Options
    BOOL CrossNodeAtomicShaderInstructions = features.CrossNodeAtomicShaderInstructions();

    // 27: Options5
    BOOL SRVOnlyTiledResourceTier3 = features.SRVOnlyTiledResourceTier3();
    D3D12_RENDER_PASS_TIER RenderPassesTier = features.RenderPassesTier();
    D3D12_RAYTRACING_TIER RaytracingTier = features.RaytracingTier();

    // 28: Displayable
    BOOL DisplayableTexture = features.DisplayableTexture();

    // 30: Options6
    BOOL AdditionalShadingRatesSupported = features.AdditionalShadingRatesSupported();
    BOOL PerPrimitiveShadingRateSupportedWithViewportIndexing = features.PerPrimitiveShadingRateSupportedWithViewportIndexing();
    D3D12_VARIABLE_SHADING_RATE_TIER VariableShadingRateTier = features.VariableShadingRateTier();
    BOOL ShadingRateImageTileSize = features.ShadingRateImageTileSize();
    BOOL BackgroundProcessingSupported = features.BackgroundProcessingSupported();

    // 31: Query Metacommand

    // 32: Options7
    D3D12_MESH_SHADER_TIER MeshShaderTier = features.MeshShaderTier();
    D3D12_SAMPLER_FEEDBACK_TIER SamplerFeedbackTier = features.SamplerFeedbackTier();

    // 33: Session Type Count
    UINT SessionTypeCount = features.ProtectedResourceSessionTypeCount();
    
    // 34: Session Types
    std::vector<GUID> SessionTypes = features.ProtectedResourceSessionTypes();

    // 36: Options8
    BOOL UnalignedBlockTexturesSupported = features.UnalignedBlockTexturesSupported();

    // 37: Options9
    BOOL MeshShaderPipelineStatsSupported = features.MeshShaderPipelineStatsSupported();
    BOOL MeshShaderSupportsFullRangeRenderTargetArrayIndex = features.MeshShaderSupportsFullRangeRenderTargetArrayIndex();
    D3D12_WAVE_MMA_TIER WaveMMATier = features.WaveMMATier();

    // 39: Options10
    BOOL VariableRateShadingSumCombinerSupported = features.VariableRateShadingSumCombinerSupported();
    BOOL MeshShaderPerPrimitiveShadingRateSupported = features.MeshShaderPerPrimitiveShadingRateSupported();

    // 40: Options11
    BOOL AtomicInt64OnDescriptorHeapResourceSupported = features.AtomicInt64OnDescriptorHeapResourceSupported();

    return 0;
}
