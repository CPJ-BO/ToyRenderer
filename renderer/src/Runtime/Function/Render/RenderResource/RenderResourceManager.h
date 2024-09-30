#pragma once

#include "Core/Util/IndexAlloctor.h"
#include "Function/Global/Definations.h"
#include "Function/Render/RHI/RHIStructs.h"
#include "Function/Render/RenderResource/RenderStructs.h"
#include "Buffer.h"
#include "Function/Render/RenderResource/Sampler.h"
#include "Function/Render/RenderResource/Texture.h"
#include "RenderStructs.h"

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

typedef struct BindlessResourceInfo
{
    ResourceType resourceType = RESOURCE_TYPE_NONE;

	RHIBufferRef buffer;
	RHITextureViewRef textureView;
	RHISamplerRef sampler;
	//TODO 光追加速结构

	uint64_t bufferOffset = 0;	// 仅buffer使用
	uint64_t bufferRange = 0;
} BindlessResourceInfo;

enum BindlessSlot
{
    BINDLESS_SLOT_POSITION = 0,
    BINDLESS_SLOT_NORMAL,
    BINDLESS_SLOT_TANGENT,
    BINDLESS_SLOT_TEXCOORD,
    BINDLESS_SLOT_COLOR,
    BINDLESS_SLOT_BONE_INDEX,
    BINDLESS_SLOT_BONE_WEIGHT,
    BINDLESS_SLOT_ANIMATION,
    BINDLESS_SLOT_INDEX,

    BINDLESS_SLOT_SAMPLER,
    BINDLESS_SLOT_TEXTURE_1D,
    BINDLESS_SLOT_TEXTURE_1D_ARRAY,
    BINDLESS_SLOT_TEXTURE_2D,
    BINDLESS_SLOT_TEXTURE_2D_ARRAY,
    BINDLESS_SLOT_TEXTURE_CUBE,
    BINDLESS_SLOT_TEXTURE_3D,

    BINDLESS_SLOT_MAX_ENUM,     //
};

enum PerFrameBindingID
{
	PER_FRAME_BINDING_GLOBAL_SETTING = 0,

	PER_FRAME_BINDING_CAMERA,
	PER_FRAME_BINDING_OBJECT,
    PER_FRAME_BINDING_MATERIAL,
    PER_FRAME_BINDING_LIGHT,
    PER_FRAME_BINDING_LIGHT_CLUSTER_GRID,
    PER_FRAME_BINDING_LIGHT_CLUSTER_INDEX,
    PER_FRAME_BINDING_DIRECTIONAL_SHADOW,
    PER_FRAME_BINDING_POINT_SHADOW,
    PER_FRAME_BINDING_MESH_CLUSTER,
    PER_FRAME_BINDING_MESH_CLUSTER_GROUP,
    PER_FRAME_BINDING_MESH_CLUSTER_DRAW_INFO,
    PER_FRAME_BINDING_DEPTH,
    PER_FRAME_BINDING_DEPTH_PYRAMID,
    PER_FRAME_BINDING_VELOCITY,
    PER_FRAME_BINDING_VERTEX,
    
	PER_FRAME_BINDING_BINDLESS_POSITION,
    PER_FRAME_BINDING_BINDLESS_NORMAL,
    PER_FRAME_BINDING_BINDLESS_TANGENT,
    PER_FRAME_BINDING_BINDLESS_TEXCOORD,
    PER_FRAME_BINDING_BINDLESS_COLOR,
    PER_FRAME_BINDING_BINDLESS_BONE_INDEX,
    PER_FRAME_BINDING_BINDLESS_BONE_WEIGHT,
    PER_FRAME_BINDING_BINDLESS_ANIMATION,
    PER_FRAME_BINDING_BINDLESS_INDEX,

    PER_FRAME_BINDING_BINDLESS_SAMPLER,
    PER_FRAME_BINDING_BINDLESS_TEXTURE_1D,
    PER_FRAME_BINDING_BINDLESS_TEXTURE_1D_ARRAY,
    PER_FRAME_BINDING_BINDLESS_TEXTURE_2D,
    PER_FRAME_BINDING_BINDLESS_TEXTURE_2D_ARRAY,
    PER_FRAME_BINDING_BINDLESS_TEXTURE_CUBE,
    PER_FRAME_BINDING_BINDLESS_TEXTURE_3D,

	PER_FRAME_BINDING_MAX_ENUM,//
};

static uint32_t BindlessSlotToPerFrameBinding(BindlessSlot slot) { return slot + (uint32_t)PER_FRAME_BINDING_BINDLESS_POSITION; }


// FRenderResource是对Render线程的资源抽象，对应的FRHIResource是对RHI线程的资源抽象，有一个封装的关系
// 有相当多的子类继承
// FRenderResource也规定了一组简单的初始化和销毁用的RHI接口，对于RHI资源的操作会入队到对应线程执行（异步）
// 简单起见就不做了，只对RHI资源做必要的封装，RenderResource文件夹下的各个类作为上层渲染资源的抽象

// 主要用于bindless绑定分配等全局的资源分配和管理
class RenderResourceManager
{
public:
    RenderResourceManager() = default;
    ~RenderResourceManager() {};

    void Init();    

    void Destroy();

    uint32_t AllocateObjectID()                             { return perFrameResources[0].objectBuffer.Allocate(); }
    void ReleaseObjectID(uint32_t id)                       { perFrameResources[0].objectBuffer.Release(id); }

    uint32_t AllocateMaterialID()                           { return multiFrameResource.materialBuffer.Allocate(); }      
    void ReleaseMaterialID(uint32_t id)                     { multiFrameResource.materialBuffer.Release(id); }  
    
    uint32_t AllocatePointLightID()                         { return pointLightIDAlloctor.Allocate(); }      
    void ReleasePointLightID(uint32_t id)                   { pointLightIDAlloctor.Release(id); }  

    IndexRange AllocateMeshClusterID(uint32_t size)         { return multiFrameResource.meshClusterBuffer.Allocate(size); }      
    void ReleaseMeshClusterID(IndexRange range)             { multiFrameResource.meshClusterBuffer.Release(range); }  

    IndexRange AllocateMeshClusterGroupID(uint32_t size)    { return multiFrameResource.meshClusterGroupBuffer.Allocate(size); }      
    void ReleaseMeshClusterGroupID(IndexRange range)        { multiFrameResource.meshClusterGroupBuffer.Release(range); }  

    uint32_t AllocateVertexID()                             { return multiFrameResource.vertexBuffer.Allocate(); }      
    void ReleaseVertexID(uint32_t id)                       { multiFrameResource.vertexBuffer.Release(id); }  

    uint32_t AllocateBindlessID(const BindlessResourceInfo& resoruceInfo, BindlessSlot slot);
    void ReleaseBindlessID(uint32_t id, BindlessSlot slot);   
  
    RHITextureRef GetLightClusterGridTexture()              { return multiFrameResource.lightClusterGridTexture->texture; }
    RHITextureRef GetDirectionalShadowTexture(uint32_t id)  { return multiFrameResource.dirShadowTextures[id]->texture; }
    RHITextureRef GetPointShadowTexture(uint32_t id)        { return multiFrameResource.pointShadowTextures[id]->texture; }
    RHITextureRef GetDepthTexture()                         { return multiFrameResource.depthTexture->texture; }
    RHITextureRef GetDepthPyramidTexture(uint32_t id)       { return multiFrameResource.depthPyramidTexture[id]->texture; }
    RHITextureRef GetVelocityTexture()                      { return multiFrameResource.velocityTexture->texture; }

    void SetRenderGlobalSetting(const RenderGlobalSetting& globalSetting);
    void SetCameraInfo(const CameraInfo& cameraInfo);
    void SetObjectInfo(const ObjectInfo& objectInfo, uint32_t objectID);
    void SetDirectionalLightInfo(const DirectionalLightInfo& directionalLightInfo, uint32_t cascade);
    void SetPointLightInfo(const PointLightInfo& pointLightInfo, uint32_t pointLightID);
    void SetVolumeLightInfo(const VolumeLightInfo& volumeLightInfo, uint32_t volumeLightID);
    void SetLightSetting(const LightSetting& lightSetting);
    void SetMaterialInfo(const MaterialInfo& materialInfo, uint32_t materialID);
    void SetMeshClusterInfo(const std::vector<MeshClusterInfo>& meshClusterInfo, uint32_t baseMeshClusterID);
    void SetMeshClusterGroupInfo(const std::vector<MeshClusterGroupInfo>& meshClusterGroupInfo, uint32_t baseMeshClusterGroupID);
    void SetVertexInfo(const VertexInfo& vertexInfo, uint32_t vertexID);

    RHIShaderRef GetOrCreateRHIShader(const std::string& path, ShaderFrequency frequency, const std::string& entry = "main");  
    RHIBufferRef GetGlobalClusterDrawInfoBuffer();
    RHIBufferRef GetLightClusterIndexBuffer();

    RHIRootSignatureRef GetPerFrameRootSignature()  { return perFrameRootSignature; }
    RHIDescriptorSetRef GetPerFrameDescriptorSet();
    RHIRootSignatureRef GetSamplerRootSignature()   { return multiFrameResource.samplerRootSignature; }
    RHIDescriptorSetRef GetSamplerDescriptorSet()   { return multiFrameResource.samplerDescriptorSet; }

private:
    std::unordered_map<std::string, RHIShaderRef> shaderMap;    // 全局统一的着色器创建，假定文件路径唯一索引一个着色器

private:
    // 每帧都单独需要的资源（持续更新）
    struct PerFrameResource     
    {
        RHIDescriptorSetRef descriptorSet;

        ArrayBuffer<LightClusterIndex, LIGHT_CLUSTER_NUM * MAX_LIGHTS_PER_CLUSTER> lightClusterIndexBuffer;                     // 每帧都完全重构的buffer                              
        ArrayBuffer<MeshClusterDrawInfo, MAX_PER_FRAME_CLUSTER_SIZE * MAX_SUPPORTED_MESH_PASS_COUNT> clusterDrawInfoBuffer;     // 上一帧还在渲染时下一帧也可能在录制了，因此对每一帧有单独的buffer
    
        Buffer<CameraInfo> cameraBuffer;                                    // 有固定槽位分配的buffer，且更新频率高
        ArrayBuffer<ObjectInfo, MAX_PER_FRAME_OBJECT_SIZE> objectBuffer;
        Buffer<LightInfo> lightBuffer;
    };
    std::array<PerFrameResource, FRAMES_IN_FLIGHT> perFrameResources;

    // TODO 哪些资源需要创建多份，做cpu数据传输和gpu计算的并行处理？
    struct MultiFrameResource   
    {  
        RHIRootSignatureRef samplerRootSignature;
        RHIDescriptorSetRef samplerDescriptorSet;

        Buffer<RenderGlobalSetting> globalSettingBuffer;                    // 有固定槽位分配的buffer，且更新频率低
        ArrayBuffer<MeshClusterInfo, MAX_PER_FRAME_CLUSTER_SIZE> meshClusterBuffer;
        ArrayBuffer<MeshClusterGroupInfo, MAX_PER_FRAME_CLUSTER_SIZE> meshClusterGroupBuffer;
        ArrayBuffer<MaterialInfo, MAX_PER_FRAME_RESOURCE_SIZE> materialBuffer;
        ArrayBuffer<VertexInfo, MAX_PER_FRAME_RESOURCE_SIZE> vertexBuffer;
 
        TextureRef lightClusterGridTexture;                                 // 纹理不会有冲突
        std::array<TextureRef, DIRECTIONAL_SHADOW_CASCADE_LEVEL> dirShadowTextures;        
        std::array<TextureRef, MAX_POINT_SHADOW_COUNT> pointShadowTextures;   
        TextureRef depthTexture;
        std::array<TextureRef, 2> depthPyramidTexture;  // MIN, MAX 
        TextureRef velocityTexture;
        std::vector<SamplerRef> samplers; 
    };
    MultiFrameResource multiFrameResource;

    // 全部的资源索引分配器
    std::array<IndexAlloctor, BINDLESS_SLOT_MAX_ENUM> bindlessIDAlloctor;
    IndexAlloctor pointLightIDAlloctor;
    IndexAlloctor volumeLightInfoIDAlloctor;

    // 逐帧资源的根签名
    RHIRootSignatureRef perFrameRootSignature;

    void InitGlobalResources(); // 负责创建全局（帧间）的渲染资源
};

void SetMeshClusterGroupInfo(const std::vector<MeshClusterGroupInfo>& meshClusterGroupInfo, uint32_t baseMeshClusterGroupID);  