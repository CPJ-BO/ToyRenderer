#pragma once

#include "Function/Render/RHI/RHIStructs.h"
#include "MurmurHash2.h"

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <unordered_map>

// RDG所用到的主要的资源，由于每帧重构，都需要池化
// 包括buffer texture textureView等
// 录制每个pass的命令时会申请此处的实际RHI资源，录制完成后再将资源归还给池子

// renderPass和frameBuffer是在RHI层实现的池化
// TODO 目前并没有做池化后的GC，冗余资源没有定期删除

class RDGBufferPool
{
public:
    struct PooledBuffer
    {
        RHIBufferRef buffer;    // RHIBuffer的析构是在RHI里追踪完成的，无须手动释放
        RHIResourceState state; // 当前所处的资源状态
    };

    struct Key
    {
        Key(const RHIBufferInfo& info) 
        : memoryUsage(info.memoryUsage)
        , type(info.type)
        , creationFlag(info.creationFlag)
        {}
  
        // uint64_t size;  // size不作为键，只要大小够就行

        MemoryUsage memoryUsage;
        ResourceType type;

        BufferCreationFlags creationFlag;

        friend bool operator== (const Key& a, const Key& b)
        {
            return  a.memoryUsage == b.memoryUsage &&
                    a.type == b.type &&
                    a.creationFlag == b.creationFlag;
        }

        struct Hash {
            size_t operator()(const Key& a) const {
                return MurmurHash64A(&a, sizeof(Key), 0);        
            }
        };
    };

    PooledBuffer Allocate(const RHIBufferInfo& info);
    void Release(const PooledBuffer& pooledBuffer);

    inline uint32_t PooledSize()    { return pooledSize; };
    inline uint32_t AllocatedSize() { return allocatedSize; }
    void Clear()                    { pooledBuffers.clear(); pooledSize = 0; }

    static std::shared_ptr<RDGBufferPool> Get()
    {
        static std::shared_ptr<RDGBufferPool> pool;
        if(pool == nullptr) pool = std::make_shared<RDGBufferPool>();
        return pool;
    }

private:
    std::unordered_map<Key, std::list<PooledBuffer>, Key::Hash> pooledBuffers;    // 可能有多个满足需求的buffer，还得再靠size等筛选
    uint32_t pooledSize = 0;
    uint32_t allocatedSize = 0;
};


class RDGTexturePool
{
public:
    struct PooledTexture
    {
        RHITextureRef texture;  
        RHIResourceState state; 
    };

    struct Key
    {
        Key(const RHITextureInfo& info) 
        : info(info)
        {}

        RHITextureInfo info;

        friend bool operator== (const Key& a, const Key& b)
        {
            return  a.info == b.info;
        }

        struct Hash {
            size_t operator()(const Key& a) const {
                return MurmurHash64A(&a, sizeof(Key), 0);  
            }
        };
    };

    PooledTexture Allocate(const RHITextureInfo& info);
    void Release(const PooledTexture& pooledTexture);

    inline uint32_t PooledSize()    { return pooledSize; };
    inline uint32_t AllocatedSize() { return allocatedSize; }
    void Clear()                    { pooledTextures.clear(); pooledSize = 0; }

    static std::shared_ptr<RDGTexturePool> Get()
    {
        static std::shared_ptr<RDGTexturePool> pool;
        if(pool == nullptr) pool = std::make_shared<RDGTexturePool>();
        return pool;
    }

private:
    std::unordered_map<Key, std::list<PooledTexture>, Key::Hash> pooledTextures;  
    uint32_t pooledSize = 0;
    uint32_t allocatedSize = 0;
};


class RDGTextureViewPool
{
public:
    struct PooledTextureView
    {
        RHITextureViewRef textureView;  
    };

    struct Key
    {
        Key(const RHITextureViewInfo& info) 
        : info(info)
        {}

        RHITextureViewInfo info;	

        friend bool operator== (const Key& a, const Key& b)
        {
            return  a.info == b.info;
        }

        struct Hash {
            size_t operator()(const Key& a) const {
                return MurmurHash64A(&a.info, sizeof(RHITextureViewInfo), 0);
            }
        };
    };

    PooledTextureView Allocate(const RHITextureViewInfo& info);
    void Release(const PooledTextureView& pooledTextureView);

    inline uint32_t PooledSize()    { return pooledSize; }
    inline uint32_t AllocatedSize() { return allocatedSize; }
    void Clear()                    { pooledTextureViews.clear(); pooledSize = 0; }

    static std::shared_ptr<RDGTextureViewPool> Get()
    {
        static std::shared_ptr<RDGTextureViewPool> pool;
        if(pool == nullptr) pool = std::make_shared<RDGTextureViewPool>();
        return pool;
    }

private:
    std::unordered_map<Key, std::list<PooledTextureView>, Key::Hash> pooledTextureViews;
    uint32_t pooledSize = 0;
    uint32_t allocatedSize = 0;
};

class RDGDescriptorSetPool
{
public:
    struct PooledDescriptor
    {
        RHIDescriptorSetRef descriptor;  
    };

    struct Key
    {
        Key(const RHIRootSignatureInfo& info, uint32_t set) 
        : entries(info.GetEntries())
        , set(set)
        {}

        std::vector<ShaderResourceEntry> entries;
        uint32_t set;	

        friend bool operator== (const Key& a, const Key& b)
        {
            return  a.entries == b.entries&& 
                    a.set == b.set;
        }

        struct HashEntries {
            size_t operator()(std::vector<ShaderResourceEntry> entries) const {
                return MurmurHash64A(entries.data(), entries.size() * sizeof(ShaderResourceEntry), 0);  
            }
        };

        struct Hash {
            size_t operator()(const Key& a) const {
                return  std::hash<uint32_t>()(a.set) ^
                        (HashEntries()(a.entries) << 1);
            }
        };
    };

    PooledDescriptor Allocate(const RHIRootSignatureRef& rootSignature, uint32_t set);
    void Release(const PooledDescriptor& pooledDescriptor, const RHIRootSignatureRef& rootSignature, uint32_t set);

    inline uint32_t PooledSize()    { return pooledSize; }
    inline uint32_t AllocatedSize() { return allocatedSize; }
    void Clear()                    { pooledDescriptors.clear(); pooledSize = 0; }

    static std::shared_ptr<RDGDescriptorSetPool> Get()
    {
        static std::shared_ptr<RDGDescriptorSetPool> pool;
        if(pool == nullptr) pool = std::make_shared<RDGDescriptorSetPool>();
        return pool;
    }

private:
    std::unordered_map<Key, std::list<PooledDescriptor>, Key::Hash> pooledDescriptors;
    uint32_t pooledSize = 0;
    uint32_t allocatedSize = 0;
};
