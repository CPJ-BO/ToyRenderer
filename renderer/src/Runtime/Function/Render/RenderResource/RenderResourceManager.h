#pragma once

#include "Core/Util/IndexAlloctor.h"
#include "Function/Global/Definations.h"
#include "Function/Render/RHI/RHIStructs.h"
#include "Function/Render/RenderResource/RenderStructs.h"
#include "Buffer.h"

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>

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
    // TODO cluster

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

	PER_FRAME_BINDING_BINDLESS_POSITION,
    PER_FRAME_BINDING_BINDLESS_NORMAL,
    PER_FRAME_BINDING_BINDLESS_TANGENT,
    PER_FRAME_BINDING_BINDLESS_TEXCOORD,
    PER_FRAME_BINDING_BINDLESS_COLOR,
    PER_FRAME_BINDING_BINDLESS_BONE_INDEX,
    PER_FRAME_BINDING_BINDLESS_BONE_WEIGHT,
    PER_FRAME_BINDING_BINDLESS_ANIMATION,
    PER_FRAME_BINDING_BINDLESS_INDEX,

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
// 简单起见就不做了，这里只对RHI资源做必要的封装

// 主要用于bindless绑定分配等全局的资源分配和管理
class RenderResourceManager
{
public:
    RenderResourceManager() = default;
    ~RenderResourceManager() {};

    void Init();    

    void Destroy();

    RHIShaderRef GetOrCreateRHIShader(const std::string& path, ShaderFrequency frequency, const std::string& entry = "main");    

    uint32_t AllocateObjectID()             { return perFrameResources[0].objectBuffer.Allocate(); }
    void ReleaseObjectID(uint32_t id)       { perFrameResources[0].objectBuffer.Release(id); }

    uint32_t AllocateMaterialID()           { return perFrameResources[0].materialBuffer.Allocate(); }      
    void ReleaseMaterialID(uint32_t id)     { perFrameResources[0].materialBuffer.Release(id); }  
    
    uint32_t AllocateBindlessID(const BindlessResourceInfo& resoruceInfo, BindlessSlot slot);
    void ReleaseBindlessID(uint32_t id, BindlessSlot slot);   

    void SetCameraInfo(const CameraInfo& cameraInfo);
    void SetObjectInfo(const ObjectInfo& objectInfo, uint32_t objectID);
    void SetMaterialInfo(const MaterialInfo& materialInfo, uint32_t materialID);

    RHIRootSignatureRef GetPerFrameRootSignature() { return perFrameRootSignature; }
    RHIDescriptorSetRef GetPerFrameDescriptorSet();

private:
    std::unordered_map<std::string, RHIShaderRef> shaderMap;    // 全局统一的着色器创建，假定文件路径唯一索引一个着色器

private:
    struct PerFrameResource 
    {
        RHIDescriptorSetRef descriptorSet;

        Buffer<CameraInfo> cameraBuffer;
        ArrayBuffer<ObjectInfo, MAX_PER_FRAME_RESOURCE_SIZE> objectBuffer;
        ArrayBuffer<MaterialInfo, MAX_PER_FRAME_RESOURCE_SIZE> materialBuffer;
    };

    void InitGlobalResources(); // 负责创建全局（帧间）的渲染资源

    RHIRootSignatureRef perFrameRootSignature;
    std::array<PerFrameResource, FRAMES_IN_FLIGHT> perFrameResources;

    std::array<IndexAlloctor, BINDLESS_SLOT_MAX_ENUM> bindlessIDAlloctor;
};