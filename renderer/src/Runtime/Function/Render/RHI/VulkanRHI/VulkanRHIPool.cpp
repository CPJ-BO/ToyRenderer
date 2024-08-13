#include "VulkanRHIPool.h"
#include "Function/Render/RHI/VulkanRHI/VulkanRHI.h"
#include "Function/Render/RHI/VulkanRHI/VulkanUtil.h"

VkRenderPassPool::PooledRenderPass VkRenderPassPool::Allocate(const VulkanRenderPassAttachments& info)
{
    VkRenderPassPool::PooledRenderPass ret;

    auto iter = pooledPasses.find(info);
    if(iter != pooledPasses.end())
    {
        // LOG_DEBUG("VkRenderPass found in cache.");
        return iter->second;
    }

    LOG_DEBUG("VkRenderPass not found in cache, creating new.");
    ret = {
        .pass = Backend()->CreateVkRenderPass(info),             
    };
    pooledPasses[info] = ret;
    allocatedSize++;
    pooledSize++;

    return ret;
}

void VkRenderPassPool::Clear()
{
    for(auto iter : pooledPasses)
    {
        vkDestroyRenderPass(Backend()->GetLogicalDevice(), iter.second.pass, nullptr);
    }
    pooledPasses.clear();
    allocatedSize = 0;
    pooledSize = 0;
}

VkFramebufferPool::PooledFramebuffer VkFramebufferPool::Allocate(const VkFramebufferCreateInfo& info)
{
    VkFramebufferPool::PooledFramebuffer ret;

    auto iter = pooledFramebuffers.find(info);
    if(iter != pooledFramebuffers.end())
    {
        // LOG_DEBUG("VkFramebuffer found in cache.");
        return iter->second;
    }

    LOG_DEBUG("VkFramebuffer not found in cache, creating new.");
    ret = {
        .frameBuffer = Backend()->CreateVkFramebuffer(info),             
    };
    pooledFramebuffers[info] = ret;
    allocatedSize++;
    pooledSize++;

    return ret;
}

void VkFramebufferPool::Clear()
{
    for(auto iter : pooledFramebuffers)
    {
        vkDestroyFramebuffer(Backend()->GetLogicalDevice(), iter.second.frameBuffer, nullptr);
    }
    pooledFramebuffers.clear();
    allocatedSize = 0;
    pooledSize = 0;
}