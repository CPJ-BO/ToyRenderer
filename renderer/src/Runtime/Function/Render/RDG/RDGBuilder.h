#pragma once

#include "Core/DependencyGraph/DependencyGraph.h"
#include "Core/Log/Log.h"
#include "Function/Render/RDG/RDGHandle.h"
#include "Function/Render/RDG/RDGNode.h"
#include "Function/Render/RHI/RHICommandList.h"
#include "Function/Render/RHI/RHIStructs.h"

#include <cstdint>
#include <string>
#include <vector>

class RDGBlackBoard
{
public:
    RDGPassNodeRef Pass(std::string name);
    RDGBufferNodeRef Buffer(std::string name);
    RDGTextureNodeRef Texture(std::string name);

    void AddPass(RDGPassNodeRef pass);
    void AddBuffer(RDGBufferNodeRef buffer);
    void AddTexture(RDGTextureNodeRef texture);

    void Clear() { passes.clear(); buffers.clear(); textures.clear(); }

private:
    std::map<std::string, RDGPassNodeRef> passes;
    std::map<std::string, RDGBufferNodeRef> buffers;
    std::map<std::string, RDGTextureNodeRef> textures;
};

// UE中的RDG：
// 每个pass一个cpp文件 有graphbuilder的构建回调函数，
// meshpass继承pass，多一个获取场景meshbatch的回调函数
// RDG只有单帧生命周期，其分配的资源（返回引用句柄）也只有单帧，除非声明跨帧的外部资源；资源有对象池做缓冲
// shader的参数结构体里包括对RDG资源的声明，实际上是靠shader反射做的资源绑定？避免手动在cpp里再做一次绑定
// 状态设置等信息由一个parameters结构体描述，这个结构体的生命周期也应该与RDG一致（单帧），builder会给一个allocateParameters函数来返回

// 目前的RDG只实现了最基本的功能，相当多特性还未完成，例如：
// pass排序，pass剔除，多线程录制，multi queue，资源池GC，细粒度的资源处理（内存对齐，subresource屏障等），……
class RDGBuilder
{
public:
    RDGBuilder() = default;
    RDGBuilder(RHICommandListRef command)
    : command(command)
    {}
    
    ~RDGBuilder() {};

    RDGTextureBuilder CreateTexture(std::string name);
    RDGBufferBuilder CreateBuffer(std::string name);
    RDGRenderPassBuilder CreateRenderPass(std::string name);    // 假定所有pass的添加顺序就是执行顺序，方便处理排序和依赖关系等
    RDGComputePassBuilder CreateComputePass(std::string name);     
    RDGPresentPassBuilder CreatePresentPass(std::string name);    

    RDGTextureHandle GetTexture(std::string name);
    RDGBufferHandle GetBuffer(std::string name);
    RDGRenderPassHandle GetRenderPass(std::string name)     { return GetPass<RDGRenderPassNodeRef, RDGRenderPassHandle>(name); };
    RDGComputePassHandle GetComputePass(std::string name)   { return GetPass<RDGComputePassNodeRef, RDGComputePassHandle>(name); };
    RDGPresentPassHandle GetPresentPass(std::string name)   { return GetPass<RDGPresentPassNodeRef, RDGPresentPassHandle>(name); };

    inline DependencyGraph* GetGraph() { return &graph; }

    void Execute();

private:
    void CreateBarriers(RDGPassNodeRef pass);
    void ReleaseResource(RDGPassNodeRef pass);
    void ExecutePass(RDGRenderPassNodeRef pass);
    void ExecutePass(RDGComputePassNodeRef pass);
    void ExecutePass(RDGPresentPassNodeRef pass);

    template<typename Type, typename Handle>
    Handle GetPass(std::string name)
    {
        auto node = blackBoard.Pass(name);
        if(node == nullptr) 
        {
            LOG_DEBUG("Unable to find RDG resource, please check name!");
            return Handle(UINT32_MAX);
        }
        return dynamic_cast<Type>(node)->GetHandle();
    }

    RHITextureRef Resolve(RDGTextureNodeRef textureNode);   // 从node获取到实际的RHI资源
    RHIBufferRef Resolve(RDGBufferNodeRef bufferNode);     
    void Release(RDGTextureNodeRef textureNode, RHIResourceState state);  
    void Release(RDGBufferNodeRef bufferNode, RHIResourceState state);  

    RHIResourceState PreviousState(RDGTextureNodeRef textureNode, RDGPassNodeRef passNode); // 获取当前pass（在执行顺序上）的资源的前序状态
    RHIResourceState PreviousState(RDGBufferNodeRef bufferNode, RDGPassNodeRef passNode);
    bool IsLastUsedPass(RDGTextureNodeRef textureNode, RDGPassNodeRef passNode);
    bool IsLastUsedPass(RDGBufferNodeRef bufferNode, RDGPassNodeRef passNode);

    std::vector<RDGPassNodeRef> passes; // 创建的全部pass，按照创建顺序执行

    DependencyGraph graph;
    RDGBlackBoard blackBoard;

    RHICommandListRef command;
};

class RDGTextureBuilder
{
public:
    RDGTextureBuilder(RDGBuilder* builder, RDGTextureNodeRef texture) 
    : builder(builder)
    , texture(texture) {};

    RDGTextureBuilder& Import(RHITextureRef texture, RHIResourceState initState); 
    RDGTextureBuilder& Exetent(Extent3D extent);
    RDGTextureBuilder& Format(TextureFormat format);
    RDGTextureBuilder& MemoryUsage(MemoryUsage memoryUsage);
    RDGTextureBuilder& AllowReadWrite();
    RDGTextureBuilder& AllowRenderTarget();
    RDGTextureBuilder& AllowDepthStencil();
    RDGTextureBuilder& MipLevels(uint32_t mipLevels);
    RDGTextureBuilder& ArrayLayers(uint32_t arrayLayers);

    RDGTextureHandle Finish() { return texture->GetHandle(); }

private:
    RDGBuilder* builder;
    RDGTextureNodeRef texture;
};

class RDGBufferBuilder
{
public:
    RDGBufferBuilder(RDGBuilder* builder, RDGBufferNodeRef buffer)
    : builder(builder)
    , buffer(buffer) {};

    RDGBufferBuilder& Import(RHIBufferRef buffer, RHIResourceState initState);
    RDGBufferBuilder& Size(uint32_t size);
    RDGBufferBuilder& MemoryUsage(MemoryUsage memoryUsage);
    RDGBufferBuilder& AllowVertexBuffer();
    RDGBufferBuilder& AllowIndexBuffer();
    RDGBufferBuilder& AllowReadWrite();
    RDGBufferBuilder& AllowRead();

    RDGBufferHandle Finish() { return buffer->GetHandle(); }

private:
    RDGBuilder* builder;
    RDGBufferNodeRef buffer;
};

class RDGRenderPassBuilder
{
public:
    RDGRenderPassBuilder(RDGBuilder* builder, RDGRenderPassNodeRef pass)
    : builder(builder)
    , pass(pass)
    , graph(builder->GetGraph()) {};

    RDGRenderPassBuilder& RootSignature(RHIRootSignatureRef rootSignature);                 // 若提供根签名未提供描述符，使用池化创建
    RDGRenderPassBuilder& DescriptorSet(uint32_t set, RHIDescriptorSetRef descriptorSet);   // 若提供了描述符，直接用相应的描述符
    RDGRenderPassBuilder& Read(uint32_t set, uint32_t binding, uint32_t index, RDGBufferHandle buffer, uint32_t offset = 0, uint32_t range = 0);
    RDGRenderPassBuilder& Read(uint32_t set, uint32_t binding, uint32_t index, RDGTextureHandle texture, TextureViewType viewType = VIEW_TYPE_2D, TextureSubresourceRange subresource = {});
    RDGRenderPassBuilder& Color(uint32_t binding, RDGTextureHandle texture, 
                                AttachmentLoadOp load = ATTACHMENT_LOAD_OP_DONT_CARE, 
                                AttachmentStoreOp store = ATTACHMENT_STORE_OP_DONT_CARE, 
                                Color4 clearColor = {0.0f, 0.0f, 0.0f, 0.0f}, 
                                TextureSubresourceRange subresource = {}); 
    RDGRenderPassBuilder& DepthStencil( RDGTextureHandle texture, 
                                        AttachmentLoadOp load = ATTACHMENT_LOAD_OP_DONT_CARE, 
                                        AttachmentStoreOp store = ATTACHMENT_STORE_OP_DONT_CARE, 
                                        float clearDepth = 1.0f,
	                                    uint32_t clearStencil = 0,
                                        TextureSubresourceRange subresource = {});
    RDGRenderPassBuilder& Execute(const RDGPassExecuteFunc& execute);

    RDGRenderPassHandle Finish() { return pass->GetHandle(); }

private:
    RDGBuilder* builder;
    RDGRenderPassNodeRef pass;

    DependencyGraph* graph;
};

class RDGComputePassBuilder
{
public:
    RDGComputePassBuilder(RDGBuilder* builder, RDGComputePassNodeRef pass)
    : builder(builder)
    , pass(pass)
    , graph(builder->GetGraph()) {};

    RDGComputePassBuilder& RootSignature(RHIRootSignatureRef rootSignature);                 // 若提供根签名未提供描述符，使用池化创建
    RDGComputePassBuilder& DescriptorSet(uint32_t set, RHIDescriptorSetRef descriptorSet);   // 若提供了描述符，直接用相应的描述符
    RDGComputePassBuilder& Read(uint32_t set, uint32_t binding, uint32_t index, RDGBufferHandle buffer, uint32_t offset = 0, uint32_t range = 0);
    RDGComputePassBuilder& Read(uint32_t set, uint32_t binding, uint32_t index, RDGTextureHandle texture, TextureViewType viewType = VIEW_TYPE_2D, TextureSubresourceRange subresource = {}); 
    RDGComputePassBuilder& ReadWrite(uint32_t set, uint32_t binding, uint32_t index, RDGBufferHandle buffer, uint32_t offset = 0, uint32_t range = 0);   // 好像和read也没什么区别？
    RDGComputePassBuilder& ReadWrite(uint32_t set, uint32_t binding, uint32_t index, RDGTextureHandle texture, TextureViewType viewType = VIEW_TYPE_2D, TextureSubresourceRange subresource = {}); 
    
    RDGComputePassBuilder& Execute(const RDGPassExecuteFunc& execute);

    RDGComputePassHandle Finish() { return pass->GetHandle(); }

private:
    RDGBuilder* builder;
    RDGComputePassNodeRef pass;

    DependencyGraph* graph;
};

class RDGPresentPassBuilder
{
public:
    RDGPresentPassBuilder(RDGBuilder* builder, RDGPresentPassNodeRef pass)
    : builder(builder)
    , pass(pass)
    , graph(builder->GetGraph()) {};

    RDGPresentPassHandle Finish() { return pass->GetHandle(); }

    RDGPresentPassBuilder& Texture(RDGTextureHandle texture, TextureSubresourceLayers subresource = {});
    RDGPresentPassBuilder& PresentTexture(RDGTextureHandle texture);

private:
    RDGBuilder* builder;
    RDGPresentPassNodeRef pass;

    DependencyGraph* graph;
};



