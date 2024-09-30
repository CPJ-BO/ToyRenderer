#include "RDGPool.h"
#include "Core/Log/Log.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RHI/RHIResource.h"
#include "Function/Render/RHI/RHIStructs.h"

RDGBufferPool::PooledBuffer RDGBufferPool::Allocate(const RHIBufferInfo& info)
{
    RDGBufferPool::PooledBuffer ret;

    auto& buffers = pooledBuffers[info];
    for(auto iter = buffers.begin(); iter != buffers.end(); iter++) // 搜索符合条件的，主要是size
    {
        if(iter->buffer->GetInfo().size >= info.size)
        {
            ret = *iter;
            buffers.erase(iter);
            pooledSize--;
            return ret;
        }
    }
    
    LOG_DEBUG("RHIBuffer not found in cache, creating new.");
    ret = {
        .buffer = EngineContext::RHI()->CreateBuffer(info),                 // 没找到，创建新buffer
        .state = RESOURCE_STATE_UNDEFINED
    };
    allocatedSize++;

    return ret;
}

void RDGBufferPool::Release(const RDGBufferPool::PooledBuffer& pooledBuffer)
{
    pooledBuffers[pooledBuffer.buffer->GetInfo()].push_back(pooledBuffer);
    pooledSize++;
}

RDGTexturePool::PooledTexture RDGTexturePool::Allocate(const RHITextureInfo& info)
{
    RDGTexturePool::PooledTexture ret;

    auto& textures = pooledTextures[info];
    for(auto iter = textures.begin(); iter != textures.end(); iter++) 
    {
        ret = *iter;                    // 没有什么需要筛选的，固定尺寸了
        textures.erase(iter);
        pooledSize--;
        return ret;
    }
    
    LOG_DEBUG("RHITexture not found in cache, creating new.");
    ret = {
        .texture = EngineContext::RHI()->CreateTexture(info),
        .state = RESOURCE_STATE_UNDEFINED,
    };
    allocatedSize++;

    return ret;
}

void RDGTexturePool::Release(const RDGTexturePool::PooledTexture& pooledTexture)
{
    pooledTextures[pooledTexture.texture->GetInfo()].push_back(pooledTexture);
    pooledSize++;
}

RDGTextureViewPool::PooledTextureView RDGTextureViewPool::Allocate(const RHITextureViewInfo& info)
{   
    RHITextureViewInfo actualInfo = info;   // RHI计算的时候也会用默认subresource替换，需要避免分配和返回的info不一致
    if(actualInfo.subresource.aspect == TEXTURE_ASPECT_NONE)  actualInfo.subresource = actualInfo.texture->GetDefaultSubresourceRange();    

    RDGTextureViewPool::PooledTextureView ret;

    auto& textureViews = pooledTextureViews[actualInfo];
    for(auto iter = textureViews.begin(); iter != textureViews.end(); iter++) 
    {
        ret = *iter;                    
        textureViews.erase(iter);
        pooledSize--;
        return ret;
    }
    
    LOG_DEBUG("RHITextureView not found in cache, creating new.");
    ret = {
        .textureView = EngineContext::RHI()->CreateTextureView(actualInfo)   
    };
    allocatedSize++;

    return ret;
}

void RDGTextureViewPool::Release(const RDGTextureViewPool::PooledTextureView& pooledTextureView)
{
    pooledTextureViews[pooledTextureView.textureView->GetInfo()].push_back(pooledTextureView);
    pooledSize++;
}


RDGDescriptorSetPool::PooledDescriptor RDGDescriptorSetPool::Allocate(const RHIRootSignatureRef& rootSignature, uint32_t set)
{
    RDGDescriptorSetPool::PooledDescriptor ret;

    auto& descriptors = pooledDescriptors[{rootSignature->GetInfo(), set}];
    for(auto iter = descriptors.begin(); iter != descriptors.end(); iter++) 
    {
        ret = *iter;                    
        descriptors.erase(iter);
        pooledSize--;
        return ret;
    }
    
    LOG_DEBUG("RHIDescriptorSet not found in cache, creating new.");
    ret = {
        .descriptor = rootSignature->CreateDescriptorSet(set)
    };
    allocatedSize++;

    return ret;
}

void RDGDescriptorSetPool::Release(const RDGDescriptorSetPool::PooledDescriptor& pooledDescriptor, const RHIRootSignatureRef& rootSignature, uint32_t set)
{
    pooledDescriptors[{rootSignature->GetInfo(), set}].push_back(pooledDescriptor);
    pooledSize++;    
}