#pragma once

#include "Function/Render/RHI/VulkanRHI/VulkanUtil.h"

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>
#include <volk.h>

class VkRenderPassPool
{
public:
    struct PooledRenderPass
    {
        VkRenderPass pass;    
    };

    struct Key
    {
        Key(const VulkanRenderPassAttachments& info) 
        : colorAttachments(info.colorAttachments)
        , depthStencilAttachment(info.depthStencilAttachment)
        {}
  
        std::vector<VkAttachmentDescription> colorAttachments;
        VkAttachmentDescription depthStencilAttachment = {};

        static bool Equal(const VkAttachmentDescription& a, const VkAttachmentDescription& b)
        {
            return  a.format            == b.format && 
                    a.samples           == b.samples && 
                    a.loadOp            == b.loadOp &&
                    a.storeOp           == b.storeOp;
                    // a.stencilLoadOp     == b.stencilLoadOp &&    // 后面几项全部写死 不做判断
                    // a.stencilStoreOp    == b.stencilStoreOp &&
                    // a.initialLayout     == b.initialLayout &&
                    // a.finalLayout       == b.finalLayout;
        }

        friend bool operator== (const Key& a, const Key& b)
        {
            if(a.depthStencilAttachment.format != b.depthStencilAttachment.format)    return false;
            if(a.colorAttachments.size() != b.colorAttachments.size())                return false;
            for(int i = 0; i < a.colorAttachments.size(); i++) 
                if(!Equal(a.colorAttachments[i], b.colorAttachments[i]))        return false;
            return Equal(a.depthStencilAttachment, b.depthStencilAttachment);
        } 

        struct HashDescription {
            size_t operator()(const VkAttachmentDescription& a) const {
                return std::hash<uint32_t>()(a.format) ^ 
                        (std::hash<uint32_t>()(a.samples) << 1) ^
                        (std::hash<uint32_t>()(a.loadOp) << 2) ^
                        (std::hash<uint32_t>()(a.storeOp) << 3);
            }
        };

        struct HashVectorDescription {
            size_t operator()(const std::vector<VkAttachmentDescription>& a) const {
                size_t seed = 0;
                for (const VkAttachmentDescription& i : a) {
                    seed ^= HashDescription()(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                }
                return seed;
            }
        };

        struct Hash {
            size_t operator()(const Key& a) const {
                return HashVectorDescription()(a.colorAttachments) ^ 
                        (HashDescription()(a.depthStencilAttachment) << 1);
            }
        };
    };

    ~VkRenderPassPool() { Clear(); }

    PooledRenderPass Allocate(const VulkanRenderPassAttachments& info);
    // void Release(const PooledRenderPass& pooledPass);

    inline uint32_t PooledSize()    { return pooledSize; }
    inline uint32_t AllocatedSize() { return allocatedSize; }
    void Clear();

private:
    std::unordered_map<Key, PooledRenderPass, Key::Hash> pooledPasses;    
    uint32_t pooledSize = 0;
    uint32_t allocatedSize = 0;
};

class VkFramebufferPool
{
public:
    struct PooledFramebuffer
    {
        VkFramebuffer frameBuffer;    
    };

    struct Key
    {
        Key(const VkFramebufferCreateInfo& info) 
        : sType(info.sType)
        , pNext(info.pNext)
        , flags(info.flags)
        , renderPass(info.renderPass)
        , attachmentCount(info.attachmentCount)
        , width(info.width)
        , height(info.height)
        , layers(info.layers)
        {
            for(uint32_t i = 0; i < info.attachmentCount; i++) { pAttachments.push_back(info.pAttachments[i]); }
        }
  
        VkStructureType             sType;
        const void*                 pNext;
        VkFramebufferCreateFlags    flags;
        VkRenderPass                renderPass;
        uint32_t                    attachmentCount;          
        std::vector<VkImageView>    pAttachments;       // 不能用指针，得存一份
        uint32_t                    width;
        uint32_t                    height;
        uint32_t                    layers;


        friend bool operator== (const Key& a, const Key& b)
        {
            if(a.attachmentCount != b.attachmentCount) return false;
            for (uint32_t i = 0; i < a.attachmentCount; i++) {
                if(a.pAttachments[i] != b.pAttachments[i]) return false;
            }

            return  a.renderPass == b.renderPass &&
                    a.width == b.width &&
                    a.height == b.height &&
                    a.layers == b.layers;
        } 

        struct HashAttachments {
            size_t operator()(std::vector<VkImageView> pAttachments) const {
                size_t ret = 0;
                for (uint32_t i = 0; i < pAttachments.size(); i++) {
                    ret ^= std::hash<uint64_t>()((uint64_t)pAttachments[i]);
                    ret = ret << 1;
                }
                return ret;
            }
        };

        struct Hash {
            size_t operator()(const Key& a) const {
                return HashAttachments()(a.pAttachments) ^ 
                        (std::hash<uint64_t>()((uint64_t)a.renderPass) << 1 ) ^
                        (std::hash<uint32_t>()(a.width) << 2) ^
                        (std::hash<uint32_t>()(a.height) << 3) ^
                        (std::hash<uint32_t>()(a.layers) << 4);
            }
        };
    };

    ~VkFramebufferPool() { Clear(); }

    PooledFramebuffer Allocate(const VkFramebufferCreateInfo& info);
    // void Release(const PooledFramebuffer& pooledFramebuffer);

    inline uint32_t PooledSize()    { return pooledSize; }
    inline uint32_t AllocatedSize() { return allocatedSize; }
    void Clear();

private:
    std::unordered_map<Key, PooledFramebuffer, Key::Hash> pooledFramebuffers;    
    uint32_t pooledSize = 0;
    uint32_t allocatedSize = 0;
};