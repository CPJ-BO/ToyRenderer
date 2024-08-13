#include "RenderResourceManager.h"
#include "Core/Util/IndexAlloctor.h"
#include "Function/Global/Definations.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RHI/RHIStructs.h"

#include <cstdint>

void RenderResourceManager::Init()
{
    for(auto& alloctor : bindlessIDAlloctor) alloctor = IndexAlloctor(MAX_BINDLESS_RESOURCE_SIZE);

    InitGlobalResources();
}  

void RenderResourceManager::Destroy()
{
    
}

RHIShaderRef RenderResourceManager::GetOrCreateRHIShader(const std::string& path, ShaderFrequency frequency, const std::string& entry)
{
    auto iter = shaderMap.find(path);
    if(iter != shaderMap.end()) return iter->second;
    
    std::vector<uint8_t> code;
    EngineContext::File()->LoadBinary(path, code);

    RHIShaderInfo shaderInfo = {
        .entry = entry,
        .frequency = frequency,
        .code = code
    };
    RHIShaderRef shader = EngineContext::RHI()->CreateShader(shaderInfo);

    shaderMap.emplace(path, shader);
    return shader;
}

uint32_t RenderResourceManager::AllocateBindlessID(const BindlessResourceInfo& resoruceInfo, BindlessSlot slot)
{
    uint32_t index = bindlessIDAlloctor[slot].Allocate();

    for(auto& resource : perFrameResources)
    {
        resource.descriptorSet->UpdateDescriptor({
        .binding = BindlessSlotToPerFrameBinding(slot),
        .index = index,
        .resourceType = resoruceInfo.resourceType,
        .buffer = resoruceInfo.buffer,
        .textureView = resoruceInfo.textureView,
        .sampler = resoruceInfo.sampler,
        .bufferOffset = resoruceInfo.bufferOffset,
        .bufferRange = resoruceInfo.bufferRange});
    }

    return index;
}

void RenderResourceManager::ReleaseBindlessID(uint32_t id, BindlessSlot slot)
{
    bindlessIDAlloctor[slot].Release(id);
}

void RenderResourceManager::SetCameraInfo(const CameraInfo& cameraInfo)
{
    perFrameResources[EngineContext::CurrentFrameIndex()].cameraBuffer.SetData(cameraInfo);
}

void RenderResourceManager::SetObjectInfo(const ObjectInfo& objectInfo, uint32_t objectID)
{
    perFrameResources[EngineContext::CurrentFrameIndex()].objectBuffer.SetData(objectInfo, objectID);
}

void RenderResourceManager::SetMaterialInfo(const MaterialInfo& materialInfo, uint32_t materialID)
{
    perFrameResources[EngineContext::CurrentFrameIndex()].materialBuffer.SetData(materialInfo, materialID);
}

RHIDescriptorSetRef RenderResourceManager::GetPerFrameDescriptorSet() 
{ 
    return perFrameResources[EngineContext::CurrentFrameIndex()].descriptorSet; 
}

void RenderResourceManager::InitGlobalResources()
{
    RHIRootSignatureInfo info = {};
    info.AddEntry({0, PER_FRAME_BINDING_GLOBAL_SETTING, 1, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_CAMERA, 1, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_OBJECT, 1, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_MATERIAL, 1, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_POSITION, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_NORMAL, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_TANGENT, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_TEXCOORD, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_COLOR, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_BONE_INDEX, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_BONE_WEIGHT, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_ANIMATION, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_INDEX, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_TEXTURE_1D, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_TEXTURE_1D_ARRAY, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_TEXTURE_2D, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_TEXTURE_2D_ARRAY, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_TEXTURE_CUBE, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_TEXTURE_3D, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER});

    perFrameRootSignature = EngineContext::RHI()->CreateRootSignature(info);
    for(auto& resource : perFrameResources)
    {
        resource.descriptorSet = perFrameRootSignature->CreateDescriptorSet(0);

        resource.descriptorSet->UpdateDescriptor({
        .binding = PER_FRAME_BINDING_CAMERA,
        .index = 0,
        .resourceType = RESOURCE_TYPE_RW_BUFFER,
        .buffer = resource.cameraBuffer.buffer});

        resource.descriptorSet->UpdateDescriptor({
        .binding = PER_FRAME_BINDING_OBJECT,
        .index = 0,
        .resourceType = RESOURCE_TYPE_RW_BUFFER,
        .buffer = resource.objectBuffer.buffer});

        resource.descriptorSet->UpdateDescriptor({
        .binding = PER_FRAME_BINDING_MATERIAL,
        .index = 0,
        .resourceType = RESOURCE_TYPE_RW_BUFFER,
        .buffer = resource.materialBuffer.buffer});
    }


}