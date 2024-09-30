#include "Buffer.h"
#include "Core/Math/Math.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RHI/RHIStructs.h"
#include "Function/Render/RenderResource/RenderResourceManager.h"
#include "RenderStructs.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

RHIBackendRef GlobalRHIBackend()
{
    return EngineContext::RHI();
}

VertexBuffer::VertexBuffer()
{
    vertexID = EngineContext::RenderResource()->AllocateVertexID();
}

VertexBuffer::~VertexBuffer()
{
    if(!EngineContext::Destroyed())
    {
        if(vertexInfo.positionID != 0)     EngineContext::RenderResource()->ReleaseBindlessID(vertexInfo.positionID, BINDLESS_SLOT_POSITION);   
        if(vertexInfo.normalID != 0)       EngineContext::RenderResource()->ReleaseBindlessID(vertexInfo.normalID, BINDLESS_SLOT_NORMAL);  
        if(vertexInfo.tangentID != 0)      EngineContext::RenderResource()->ReleaseBindlessID(vertexInfo.tangentID, BINDLESS_SLOT_TANGENT);  
        if(vertexInfo.texCoordID != 0)     EngineContext::RenderResource()->ReleaseBindlessID(vertexInfo.texCoordID, BINDLESS_SLOT_TEXCOORD);  
        if(vertexInfo.colorID != 0)        EngineContext::RenderResource()->ReleaseBindlessID(vertexInfo.colorID, BINDLESS_SLOT_COLOR);  
        if(vertexInfo.boneIndexID != 0)    EngineContext::RenderResource()->ReleaseBindlessID(vertexInfo.boneIndexID, BINDLESS_SLOT_BONE_INDEX); 
        if(vertexInfo.boneWeightID != 0)   EngineContext::RenderResource()->ReleaseBindlessID(vertexInfo.boneWeightID, BINDLESS_SLOT_BONE_WEIGHT);   

        if(vertexID != 0)       EngineContext::RenderResource()->ReleaseVertexID(vertexID);
    }
}

void VertexBuffer::SetBufferData(void* data, uint32_t size, RHIBufferRef& buffer, uint32_t& id, uint32_t slot)
{
    if(size == 0) return;
    if(!buffer || buffer->GetInfo().size < size)  // 创建buffer
    {
        buffer = EngineContext::RHI()->CreateBuffer({
        .size = size,
        .memoryUsage = MEMORY_USAGE_CPU_TO_GPU,
        .type = RESOURCE_TYPE_RW_BUFFER | RESOURCE_TYPE_VERTEX_BUFFER,
        .creationFlag = BUFFER_CREATION_PERSISTENT_MAP});

        if(id != 0) EngineContext::RenderResource()->ReleaseBindlessID(id, (BindlessSlot)slot);
        id = EngineContext::RenderResource()->AllocateBindlessID({
            .resourceType = RESOURCE_TYPE_RW_BUFFER,
            .buffer = buffer,
            .bufferOffset = 0,
            .bufferRange = size}, 
            (BindlessSlot)slot);
    }
    // if(!stagingBuffer || stagingBuffer->GetInfo().size < size)
    // {
    //     stagingBuffer = EngineContext::RHI()->CreateBuffer({
    //     .size = size,
    //     .memoryUsage = MEMORY_USAGE_CPU_TO_GPU,
    //     .type = RESOURCE_TYPE_RW_BUFFER,      
    //     .creationFlag = BUFFER_CREATION_PERSISTENT_MAP});
    // }

    memcpy(buffer->Map(), data, size);
    // buffer->UnMap();
    // EngineContext::RHI()->GetImmediateCommand()->CopyBuffer(stagingBuffer, 0, buffer, 0, size);

    EngineContext::RenderResource()->SetVertexInfo(vertexInfo, vertexID);   // 最后还要更新vertexStream的信息
}

void VertexBuffer::SetPosition(const std::vector<Vec3>& position)
{
    SetBufferData(
        (void*)position.data(), 
        position.size() * sizeof(Vec3),
        positionBuffer, 
        vertexInfo.positionID, 
        BINDLESS_SLOT_POSITION);
    
    vertexNum = position.size();    // 存一下当前的顶点数目，以position为基准
}

void VertexBuffer::SetNormal(const std::vector<Vec3>& normal)
{
    SetBufferData(
        (void*)normal.data(), 
        normal.size() * sizeof(Vec3),
        normalBuffer, 
        vertexInfo.normalID, 
        BINDLESS_SLOT_NORMAL);
}

void VertexBuffer::SetTangent(const std::vector<Vec4>& tangent)
{
    SetBufferData(
        (void*)tangent.data(), 
        tangent.size() * sizeof(Vec4),
        tangentBuffer, 
        vertexInfo.tangentID, 
        BINDLESS_SLOT_TANGENT);
}

void VertexBuffer::SetTexCoord(const std::vector<Vec2>& texCoord)
{
    SetBufferData(
        (void*)texCoord.data(), 
        texCoord.size() * sizeof(Vec2),
        texCoordBuffer, 
        vertexInfo.texCoordID, 
        BINDLESS_SLOT_TEXCOORD);
}

void VertexBuffer::SetColor(const std::vector<Vec3>& color)
{
    SetBufferData(
        (void*)color.data(), 
        color.size() * sizeof(Vec3),
        colorBuffer, 
        vertexInfo.colorID, 
        BINDLESS_SLOT_COLOR);
}

void VertexBuffer::SetBoneIndex(const std::vector<IVec4>& boneIndex)
{
    SetBufferData(
        (void*)boneIndex.data(), 
        boneIndex.size() * sizeof(IVec4),
        boneIndexBuffer, 
        vertexInfo.boneIndexID, 
        BINDLESS_SLOT_BONE_INDEX);
}

void VertexBuffer::SetBoneWeight(const std::vector<Vec4>& boneWeight)
{
    SetBufferData(
        (void*)boneWeight.data(), 
        boneWeight.size() * sizeof(Vec4),
        boneWeightBuffer, 
        vertexInfo.boneWeightID, 
        BINDLESS_SLOT_BONE_WEIGHT);
}

IndexBuffer::~IndexBuffer()
{
    if(!EngineContext::Destroyed() && indexID != 0) EngineContext::RenderResource()->ReleaseBindlessID(indexID, BINDLESS_SLOT_INDEX);
}

void IndexBuffer::SetIndex(const std::vector<uint32_t>& index)
{
    indexNum = index.size();
    uint32_t size = index.size() * sizeof(uint32_t);

    if(size == 0) return;
    if(!buffer || buffer->GetInfo().size < size)  // 创建buffer
    {
        buffer = EngineContext::RHI()->CreateBuffer({
        .size = size,
        .memoryUsage = MEMORY_USAGE_CPU_TO_GPU,
        .type = RESOURCE_TYPE_RW_BUFFER | RESOURCE_TYPE_INDEX_BUFFER,      
        .creationFlag = BUFFER_CREATION_PERSISTENT_MAP});

        if(indexID != 0) EngineContext::RenderResource()->ReleaseBindlessID(indexID, BINDLESS_SLOT_INDEX);
        indexID = EngineContext::RenderResource()->AllocateBindlessID({
            .resourceType = RESOURCE_TYPE_RW_BUFFER,
            .buffer = buffer,
            .bufferOffset = 0,
            .bufferRange = size}, 
            BINDLESS_SLOT_INDEX);
    }

    memcpy(buffer->Map(), index.data(), size);
    //buffer->UnMap();
}

