#include "RenderResourceManager.h"
#include "Core/Util/IndexAlloctor.h"
#include "Function/Global/Definations.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RHI/RHIStructs.h"
#include "Function/Render/RenderResource/RenderStructs.h"
#include "Function/Render/RenderResource/Texture.h"
#include "Sampler.h"

#include <cstdint>
#include <memory>

void RenderResourceManager::Init()
{
    for(auto& alloctor : bindlessIDAlloctor) alloctor = IndexAlloctor(MAX_BINDLESS_RESOURCE_SIZE);

    InitGlobalResources();
}  

void RenderResourceManager::Destroy()
{
    
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

void RenderResourceManager::SetRenderGlobalSetting(const RenderGlobalSetting& globalSetting)
{
    multiFrameResource.globalSettingBuffer.SetData(globalSetting);
}

void RenderResourceManager::SetCameraInfo(const CameraInfo& cameraInfo)
{
    perFrameResources[EngineContext::CurrentFrameIndex()].cameraBuffer.SetData(cameraInfo);
}

void RenderResourceManager::SetObjectInfo(const ObjectInfo& objectInfo, uint32_t objectID)
{
    perFrameResources[EngineContext::CurrentFrameIndex()].objectBuffer.SetData(objectInfo, objectID);
}

void RenderResourceManager::SetDirectionalLightInfo(const DirectionalLightInfo& directionalLightInfo, uint32_t cascade)
{
    perFrameResources[EngineContext::CurrentFrameIndex()].lightBuffer.SetData(
        &directionalLightInfo, 
        sizeof(DirectionalLightInfo), 
        DIR_LIGHT_OFFSET + cascade * sizeof(DirectionalLightInfo));
}

void RenderResourceManager::SetPointLightInfo(const PointLightInfo& pointLightInfo, uint32_t pointLightID)
{
    perFrameResources[EngineContext::CurrentFrameIndex()].lightBuffer.SetData(
        &pointLightInfo, 
        sizeof(PointLightInfo), 
        POINT_LIGHT_OFFSET + pointLightID * sizeof(PointLightInfo));
}

void RenderResourceManager::SetVolumeLightInfo(const VolumeLightInfo& volumeLightInfo, uint32_t volumeLightID)
{
    perFrameResources[EngineContext::CurrentFrameIndex()].lightBuffer.SetData(
        &volumeLightInfo, 
        sizeof(VolumeLightInfo), 
        VOLUME_LIGHT_OFFSET + volumeLightID * sizeof(VolumeLightInfo));    
}

void RenderResourceManager::SetLightSetting(const LightSetting& lightSetting)
{
    perFrameResources[EngineContext::CurrentFrameIndex()].lightBuffer.SetData(
        &lightSetting, 
        sizeof(LightSetting), 
        LIGHT_SETTING_OFFSET);
}

void RenderResourceManager::SetMaterialInfo(const MaterialInfo& materialInfo, uint32_t materialID)
{
    multiFrameResource.materialBuffer.SetData(materialInfo, materialID);
}

void RenderResourceManager::SetMeshClusterInfo(const std::vector<MeshClusterInfo>& meshClusterInfo, uint32_t baseMeshClusterID)
{
    multiFrameResource.meshClusterBuffer.SetData(meshClusterInfo, baseMeshClusterID);
}

void RenderResourceManager::SetMeshClusterGroupInfo(const std::vector<MeshClusterGroupInfo>& meshClusterGroupInfo, uint32_t baseMeshClusterGroupID)
{
    multiFrameResource.meshClusterGroupBuffer.SetData(meshClusterGroupInfo, baseMeshClusterGroupID);
}

void RenderResourceManager::SetVertexInfo(const VertexInfo& vertexInfo, uint32_t vertexID)
{
    multiFrameResource.vertexBuffer.SetData(vertexInfo, vertexID);
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

RHIBufferRef RenderResourceManager::GetGlobalClusterDrawInfoBuffer()           
{ 
    return perFrameResources[EngineContext::CurrentFrameIndex()].clusterDrawInfoBuffer.buffer; 
} 

RHIBufferRef RenderResourceManager::GetLightClusterIndexBuffer()               
{ 
    return perFrameResources[EngineContext::CurrentFrameIndex()].lightClusterIndexBuffer.buffer;
} 

RHIDescriptorSetRef RenderResourceManager::GetPerFrameDescriptorSet() 
{ 
    return perFrameResources[EngineContext::CurrentFrameIndex()].descriptorSet; 
}

void RenderResourceManager::InitGlobalResources()
{
    Extent2D windowExtent = EngineContext::Render()->GetWindowsExtent();

    RHIRootSignatureInfo info = {};
    info.AddEntry({0, PER_FRAME_BINDING_GLOBAL_SETTING, 1, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_CAMERA, 1, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_OBJECT, 1, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_MATERIAL, 1, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_LIGHT, 1, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_LIGHT_CLUSTER_GRID, 1, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_TEXTURE})
        .AddEntry({0, PER_FRAME_BINDING_LIGHT_CLUSTER_INDEX, 1, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_DIRECTIONAL_SHADOW, DIRECTIONAL_SHADOW_CASCADE_LEVEL, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_TEXTURE})
        .AddEntry({0, PER_FRAME_BINDING_POINT_SHADOW, MAX_POINT_SHADOW_COUNT, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_TEXTURE_CUBE})
        .AddEntry({0, PER_FRAME_BINDING_MESH_CLUSTER, 1, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_MESH_CLUSTER_GROUP, 1, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_MESH_CLUSTER_DRAW_INFO, 1, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_DEPTH, 1, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_TEXTURE})
        .AddEntry({0, PER_FRAME_BINDING_DEPTH_PYRAMID, 2, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_TEXTURE})
        .AddEntry({0, PER_FRAME_BINDING_VERTEX, 1, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_POSITION, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_NORMAL, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_TANGENT, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_TEXCOORD, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_COLOR, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_BONE_INDEX, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_BONE_WEIGHT, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_ANIMATION, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_INDEX, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_SAMPLER, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_SAMPLER})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_TEXTURE_1D, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_TEXTURE})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_TEXTURE_1D_ARRAY, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_TEXTURE})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_TEXTURE_2D, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_TEXTURE})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_TEXTURE_2D_ARRAY, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_TEXTURE})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_TEXTURE_CUBE, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_TEXTURE})
        .AddEntry({0, PER_FRAME_BINDING_BINDLESS_TEXTURE_3D, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_TEXTURE});

    perFrameRootSignature = EngineContext::RHI()->CreateRootSignature(info);
    for(auto& resource : perFrameResources) resource.descriptorSet = perFrameRootSignature->CreateDescriptorSet(0);

    // 初始化全局资源，部分直接在声明时就默认创建了
    // multiFrameResource
    {
        multiFrameResource.samplers.push_back(std::make_shared<Sampler>(
            ADDRESS_MODE_CLAMP_TO_EDGE,   
            FILTER_TYPE_LINEAR, 
            MIPMAP_MODE_LINEAR,
            0.0f));

        multiFrameResource.samplers.push_back(std::make_shared<Sampler>(
            ADDRESS_MODE_REPEAT,   
            FILTER_TYPE_LINEAR, 
            MIPMAP_MODE_LINEAR,
            0.0f));

        multiFrameResource.samplers.push_back(std::make_shared<Sampler>(
            ADDRESS_MODE_CLAMP_TO_EDGE,   
            FILTER_TYPE_LINEAR, 
            MIPMAP_MODE_NEAREST,
            0.0f,
            SAMPLER_REDUCTION_MODE_MIN));

        multiFrameResource.samplers.push_back(std::make_shared<Sampler>(
            ADDRESS_MODE_CLAMP_TO_EDGE,   
            FILTER_TYPE_LINEAR, 
            MIPMAP_MODE_NEAREST,
            0.0f,
            SAMPLER_REDUCTION_MODE_MAX));

        multiFrameResource.lightClusterGridTexture = std::make_shared<Texture>(
            TEXTURE_TYPE_3D,
            FORMAT_R32G32_UINT,
            Extent3D(LIGHT_CLUSTER_WIDTH, LIGHT_CLUSTER_HEIGHT, LIGHT_CLUSTER_DEPTH),
            1, 1);

        for(auto& texture : multiFrameResource.dirShadowTextures)
        {
            texture = std::make_shared<Texture>( 
                TEXTURE_TYPE_2D, 
                FORMAT_D32_SFLOAT,
                Extent3D(DIRECTIONAL_SHADOW_SIZE, DIRECTIONAL_SHADOW_SIZE, 1),
                1, 1);
        } 

        for(auto& texture : multiFrameResource.pointShadowTextures)
        {
            texture = std::make_shared<Texture>( 
                TEXTURE_TYPE_CUBE, 
                FORMAT_R32G32B32A32_SFLOAT,
                Extent3D(POINT_SHADOW_SIZE, POINT_SHADOW_SIZE, 1),
                6, 1);
        }

        multiFrameResource.depthTexture = std::make_shared<Texture>( 
            TEXTURE_TYPE_2D, 
            EngineContext::Render()->GetDepthFormat(),
            Extent3D(windowExtent.width, windowExtent.height, 1),
            1, 1);

        multiFrameResource.depthPyramidTexture[0] = std::make_shared<Texture>( 
            TEXTURE_TYPE_2D, 
            FORMAT_R32_SFLOAT,
            Extent3D(windowExtent.width, windowExtent.height, 1),
            1, 0);  // mip为0自动生成全mip

        multiFrameResource.depthPyramidTexture[1] = std::make_shared<Texture>( 
            TEXTURE_TYPE_2D, 
            FORMAT_R32_SFLOAT,
            Extent3D(windowExtent.width, windowExtent.height, 1),
            1, 0);  // mip为0自动生成全mip

        multiFrameResource.velocityTexture = std::make_shared<Texture>( 
            TEXTURE_TYPE_2D, 
            FORMAT_R32G32_SFLOAT,
            Extent3D(windowExtent.width, windowExtent.height, 1),
            1, 1);
    }

    // 为sampler单独创建一个描述符，方便pass使用
    {
        RHIRootSignatureInfo info = {};
        info.AddEntry({1, 0, MAX_BINDLESS_RESOURCE_SIZE, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_SAMPLER});
        multiFrameResource.samplerRootSignature = EngineContext::RHI()->CreateRootSignature(info);
        
        multiFrameResource.samplerDescriptorSet = multiFrameResource.samplerRootSignature->CreateDescriptorSet(1);
        for (uint32_t i = 0; i < multiFrameResource.samplers.size(); i++) 
        {
            multiFrameResource.samplerDescriptorSet->UpdateDescriptor({
                .binding = 0,
                .index = i,
                .resourceType = RESOURCE_TYPE_SAMPLER,
                .sampler = multiFrameResource.samplers[i]->sampler});
        }
    }

    for(auto& resource : perFrameResources)
    {
        resource.descriptorSet->UpdateDescriptor({
            .binding = PER_FRAME_BINDING_GLOBAL_SETTING,
            .index = 0,
            .resourceType = RESOURCE_TYPE_RW_BUFFER,
            .buffer = multiFrameResource.globalSettingBuffer.buffer});

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
            .buffer = multiFrameResource.materialBuffer.buffer});

        resource.descriptorSet->UpdateDescriptor({
            .binding = PER_FRAME_BINDING_LIGHT,
            .index = 0,
            .resourceType = RESOURCE_TYPE_RW_BUFFER,
            .buffer = resource.lightBuffer.buffer});

        resource.descriptorSet->UpdateDescriptor({
            .binding = PER_FRAME_BINDING_LIGHT_CLUSTER_GRID,
            .index = 0,
            .resourceType = RESOURCE_TYPE_RW_TEXTURE,
            .textureView = multiFrameResource.lightClusterGridTexture->textureView});

        resource.descriptorSet->UpdateDescriptor({
            .binding = PER_FRAME_BINDING_LIGHT_CLUSTER_INDEX,
            .index = 0,
            .resourceType = RESOURCE_TYPE_RW_BUFFER,
            .buffer = resource.lightClusterIndexBuffer.buffer});

        resource.descriptorSet->UpdateDescriptor({
            .binding = PER_FRAME_BINDING_MESH_CLUSTER,
            .index = 0,
            .resourceType = RESOURCE_TYPE_RW_BUFFER,
            .buffer = multiFrameResource.meshClusterBuffer.buffer});

        resource.descriptorSet->UpdateDescriptor({
            .binding = PER_FRAME_BINDING_MESH_CLUSTER_GROUP,
            .index = 0,
            .resourceType = RESOURCE_TYPE_RW_BUFFER,
            .buffer = multiFrameResource.meshClusterGroupBuffer.buffer});

        resource.descriptorSet->UpdateDescriptor({
            .binding = PER_FRAME_BINDING_MESH_CLUSTER_DRAW_INFO,
            .index = 0,
            .resourceType = RESOURCE_TYPE_RW_BUFFER,
            .buffer = resource.clusterDrawInfoBuffer.buffer});

        resource.descriptorSet->UpdateDescriptor({
            .binding = PER_FRAME_BINDING_DEPTH,
            .index = 0,
            .resourceType = RESOURCE_TYPE_TEXTURE,
            .textureView = multiFrameResource.depthTexture->textureView});

        resource.descriptorSet->UpdateDescriptor({
            .binding = PER_FRAME_BINDING_DEPTH_PYRAMID,
            .index = 0,
            .resourceType = RESOURCE_TYPE_TEXTURE,
            .textureView = multiFrameResource.depthPyramidTexture[0]->textureView});

        resource.descriptorSet->UpdateDescriptor({
            .binding = PER_FRAME_BINDING_DEPTH_PYRAMID,
            .index = 1,
            .resourceType = RESOURCE_TYPE_TEXTURE,
            .textureView = multiFrameResource.depthPyramidTexture[1]->textureView});

        resource.descriptorSet->UpdateDescriptor({
            .binding = PER_FRAME_BINDING_VERTEX,
            .index = 0,
            .resourceType = RESOURCE_TYPE_RW_BUFFER,
            .buffer = multiFrameResource.vertexBuffer.buffer});

        for (uint32_t i = 0; i < multiFrameResource.dirShadowTextures.size(); i++) 
        {
            resource.descriptorSet->UpdateDescriptor({
                .binding = PER_FRAME_BINDING_DIRECTIONAL_SHADOW,
                .index = i,
                .resourceType = RESOURCE_TYPE_TEXTURE,
                .textureView = multiFrameResource.dirShadowTextures[i]->textureView});
        }       

        for (uint32_t i = 0; i < multiFrameResource.pointShadowTextures.size(); i++) 
        {
            resource.descriptorSet->UpdateDescriptor({
                .binding = PER_FRAME_BINDING_POINT_SHADOW,
                .index = i,
                .resourceType = RESOURCE_TYPE_TEXTURE_CUBE,
                .textureView = multiFrameResource.pointShadowTextures[i]->textureView});
        }   

        for (uint32_t i = 0; i < multiFrameResource.samplers.size(); i++) 
        {
            resource.descriptorSet->UpdateDescriptor({
                .binding = PER_FRAME_BINDING_BINDLESS_SAMPLER,
                .index = i,
                .resourceType = RESOURCE_TYPE_SAMPLER,
                .sampler = multiFrameResource.samplers[i]->sampler});
        }
    }
}