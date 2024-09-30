#include "VulkanRHICache.h"
#include "Function/Render/RHI/VulkanRHI/VulkanRHI.h"
#include "Function/Render/RHI/VulkanRHI/VulkanUtil.h"

VkRenderPassCache::CachedRenderPass VkRenderPassCache::Allocate(const VulkanRenderPassAttachments& info)
{
    VkRenderPassCache::CachedRenderPass ret;

    auto iter = cachedPasses.find(info);
    if(iter != cachedPasses.end())
    {
        // LOG_DEBUG("VkRenderPass found in cache.");
        return iter->second;
    }

    LOG_DEBUG("VkRenderPass not found in cache, creating new.");
    ret = {
        .pass = Backend()->CreateVkRenderPass(info),             
    };
    cachedPasses[info] = ret;

    return ret;
}

void VkRenderPassCache::Clear()
{
    for(auto iter : cachedPasses)
    {
        vkDestroyRenderPass(Backend()->GetLogicalDevice(), iter.second.pass, nullptr);
    }
    cachedPasses.clear();
}

VkFramebufferCache::CachedFramebuffer VkFramebufferCache::Allocate(const VkFramebufferCreateInfo& info)
{
    VkFramebufferCache::CachedFramebuffer ret;

    auto iter = cachedFramebuffers.find(info);
    if(iter != cachedFramebuffers.end())
    {
        // LOG_DEBUG("VkFramebuffer found in cache.");
        return iter->second;
    }

    LOG_DEBUG("VkFramebuffer not found in cache, creating new.");
    ret = {
        .frameBuffer = Backend()->CreateVkFramebuffer(info),             
    };
    cachedFramebuffers[info] = ret;

    return ret;
}

void VkFramebufferCache::Clear()
{
    for(auto iter : cachedFramebuffers)
    {
        vkDestroyFramebuffer(Backend()->GetLogicalDevice(), iter.second.frameBuffer, nullptr);
    }
    cachedFramebuffers.clear();
}