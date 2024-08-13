#pragma once

#include "RHIStructs.h"

#include <cstdint>
#include <memory>
#include <vector>

struct RHICommand;

typedef struct CommandListInfo
{
	RHICommandPool* pool;
	RHICommandContextRef context;

    bool byPass = true; 		//是否立即录制

} CommandListInfo;

class RHICommandList	//CommandList没有子类，只是做DynamicRHI和RHIContext中函数的调用
{
	//直接用commandList.func()做函数声明
	//内部做判断：如果bypass就直接调内部的对应接口录制，否则就入队相应的RHICommand（每种command存有参数，并有对应接口录制的执行函数），延迟执行
	//全是胶水

public:
	RHICommandList(const CommandListInfo& info) : info(info) {}
	~RHICommandList();

	void BeginCommand();

	void EndCommand();

	void Execute(RHIFenceRef fence = nullptr, RHISemaphoreRef waitSemaphore = nullptr, RHISemaphoreRef signalSemaphore = nullptr);

    void TextureBarrier(const RHITextureBarrier& barrier);

    void BufferBarrier(const RHIBufferBarrier& barrier);

    void CopyTextureToBuffer(RHITextureRef src, TextureSubresourceLayers srcSubresource, RHIBufferRef dst, uint64_t dstOffset);

    void CopyBufferToTexture(RHIBufferRef src, uint64_t srcOffset, RHITextureRef dst, TextureSubresourceLayers dstSubresource);

    void CopyBuffer(RHIBufferRef src, uint64_t srcOffset, RHIBufferRef dst, uint64_t dstOffset, uint64_t size);

    void CopyTexture(RHITextureRef src, TextureSubresourceLayers srcSubresource, RHITextureRef dst, TextureSubresourceLayers dstSubresource);

    void GenerateMips(RHITextureRef src);

	void PushEvent(const std::string& name, Color3 color = {0.0f, 0.0f, 0.0f});

	void PopEvent();

    void BeginRenderPass(RHIRenderPassRef renderPass);

	void EndRenderPass();

    void SetViewport(Offset2D min, Offset2D max);

    void SetScissor(Offset2D min, Offset2D max);

    void SetGraphicsPipeline(RHIGraphicsPipelineRef graphicsState);

    void SetComputePipeline(RHIComputePipelineRef computeState);	

    void PushConstants(void* data, uint16_t size, ShaderFrequency frequency);

    void BindDescriptorSet(RHIDescriptorSetRef descriptor, uint32_t set = 0);

    void BindVertexBuffer(RHIBufferRef vertexBuffer, uint32_t streamIndex = 0, uint32_t offset = 0);

    void BindIndexBuffer(RHIBufferRef indexBuffer, uint32_t offset = 0);

	void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

	void DispatchIndirect(RHIBufferRef argumentBuffer, uint32_t argumentOffset = 0);

    void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);

    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, uint32_t vertexOffset = 0, uint32_t firstInstance = 0);

    void DrawIndirect(RHIBufferRef argumentBuffer, uint32_t offset, uint32_t drawCount);

    void DrawIndexedIndirect(RHIBufferRef argumentBuffer, uint32_t offset, uint32_t drawCount);

protected:
	CommandListInfo info;

    inline void AddCommand(RHICommand* command) { commands.push_back(command); }
    std::vector<RHICommand*> commands;
};
typedef std::shared_ptr<RHICommandList> RHICommandListRef;







#define ADD_COMMAND(commandName, ...) do { \
    RHICommand* command = new RHICommand##commandName(__VA_ARGS__);  \
    AddCommand(command); \
} while (0)

typedef struct RHICommand	
{
    RHICommand() = default;
    virtual ~RHICommand() = default;

    virtual void Execute(RHICommandContextRef context) = 0;
} RHICommand;

struct RHICommandBeginCommand : public RHICommand 
{
    RHICommandBeginCommand() {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandEndCommand : public RHICommand 
{
    RHICommandEndCommand() {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandTextureBarrier : public RHICommand 
{
    RHITextureBarrier barrier;

    RHICommandTextureBarrier(const RHITextureBarrier& barrier) 
    : barrier(barrier)
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandBufferBarrier : public RHICommand 
{
    RHIBufferBarrier barrier;

    RHICommandBufferBarrier(const RHIBufferBarrier& barrier)
    : barrier(barrier) 
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandCopyTextureToBuffer : public RHICommand 
{
    RHITextureRef src;
    TextureSubresourceLayers srcSubresource;
    RHIBufferRef dst;
    uint64_t dstOffset;

    RHICommandCopyTextureToBuffer(RHITextureRef src, TextureSubresourceLayers srcSubresource, RHIBufferRef dst, uint64_t dstOffset) 
    : src(src)
    , srcSubresource(srcSubresource)
    , dst(dst)
    , dstOffset(dstOffset)
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandCopyBufferToTexture : public RHICommand 
{
    RHIBufferRef src;
    uint64_t srcOffset;
    RHITextureRef dst;
    TextureSubresourceLayers dstSubresource;

    RHICommandCopyBufferToTexture(RHIBufferRef src, uint64_t srcOffset, RHITextureRef dst, TextureSubresourceLayers dstSubresource) 
    : src(src)
    , srcOffset(srcOffset)
    , dst(dst)
    , dstSubresource(dstSubresource)
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandCopyBuffer : public RHICommand 
{
    RHIBufferRef src;
    uint64_t srcOffset;
    RHIBufferRef dst;
    uint64_t dstOffset;
    uint64_t size;

    RHICommandCopyBuffer(RHIBufferRef src, uint64_t srcOffset, RHIBufferRef dst, uint64_t dstOffset, uint64_t size) 
    : src(src)
    , srcOffset(srcOffset)
    , dst(dst)
    , dstOffset(dstOffset)
    , size(size)
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandCopyTexture : public RHICommand 
{
    RHITextureRef src;
    TextureSubresourceLayers srcSubresource;
    RHITextureRef dst;
    TextureSubresourceLayers dstSubresource;

    RHICommandCopyTexture(RHITextureRef src, TextureSubresourceLayers srcSubresource, RHITextureRef dst, TextureSubresourceLayers dstSubresource)
    : src(src)
    , srcSubresource(srcSubresource)
    , dst(dst)
    , dstSubresource(dstSubresource)
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandGenerateMips : public RHICommand 
{
    RHITextureRef src;

    RHICommandGenerateMips(RHITextureRef src)
    : src(src)
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandPushEvent : public RHICommand 
{
    std::string name;
    Color3 color;

    RHICommandPushEvent(const std::string& name, Color3 color) 
    : name(name)
    , color(color)
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandPopEvent : public RHICommand 
{
    RHICommandPopEvent() {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandBeginRenderPass : public RHICommand 
{
    RHIRenderPassRef renderPass;

    RHICommandBeginRenderPass(RHIRenderPassRef renderPass) 
    : renderPass(renderPass)
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandEndRenderPass : public RHICommand 
{
    RHICommandEndRenderPass() {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandSetViewport : public RHICommand 
{
    Offset2D min;
    Offset2D max;

    RHICommandSetViewport(Offset2D min, Offset2D max)
    : min(min)
    , max(max) 
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandSetScissor : public RHICommand 
{
    Offset2D min;
    Offset2D max;

    RHICommandSetScissor(Offset2D min, Offset2D max) 
    : min(min)
    , max(max)
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandSetGraphicsPipeline : public RHICommand 
{
    RHIGraphicsPipelineRef graphicsState;

    RHICommandSetGraphicsPipeline(RHIGraphicsPipelineRef graphicsState)
    : graphicsState(graphicsState) 
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandSetComputePipeline : public RHICommand 
{
    RHIComputePipelineRef computeState;

    RHICommandSetComputePipeline(RHIComputePipelineRef computeState) 
    : computeState(computeState)
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandPushConstants : public RHICommand 
{
    void* data;
    uint16_t size;
    ShaderFrequency frequency;

    RHICommandPushConstants(void* data, uint16_t size, ShaderFrequency frequency) 
    : data(data)
    , size(size)
    , frequency(frequency)
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandBindDescriptorSet : public RHICommand 
{
    RHIDescriptorSetRef descriptor;
    uint32_t set;

    RHICommandBindDescriptorSet(RHIDescriptorSetRef descriptor, uint32_t set) 
    : descriptor(descriptor)
    , set(set)
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandBindVertexBuffer : public RHICommand 
{
    RHIBufferRef vertexBuffer;
    uint32_t streamIndex;
    uint32_t offset;

    RHICommandBindVertexBuffer(RHIBufferRef vertexBuffer, uint32_t streamIndex, uint32_t offset) 
    : vertexBuffer(vertexBuffer)
    , streamIndex(streamIndex)
    , offset(offset)
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandBindIndexBuffer : public RHICommand 
{
    RHIBufferRef indexBuffer;
    uint32_t offset;

    RHICommandBindIndexBuffer(RHIBufferRef indexBuffer, uint32_t offset) 
    : indexBuffer(indexBuffer)
    , offset(offset)
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandDispatch : public RHICommand 
{
    uint32_t groupCountX;
    uint32_t groupCountY;
    uint32_t groupCountZ;

    RHICommandDispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) 
    : groupCountX(groupCountX)
    , groupCountY(groupCountY)
    , groupCountZ(groupCountZ)
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandDispatchIndirect : public RHICommand 
{
    RHIBufferRef argumentBuffer;
    uint32_t argumentOffset;

    RHICommandDispatchIndirect(RHIBufferRef argumentBuffer, uint32_t argumentOffset) 
    : argumentBuffer(argumentBuffer)
    , argumentOffset(argumentOffset)
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandDraw : public RHICommand 
{
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;

    RHICommandDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    : vertexCount(vertexCount)
    , instanceCount(instanceCount)
    , firstVertex(firstVertex)
    ,firstInstance(firstInstance)
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandDrawIndexed : public RHICommand 
{
    uint32_t indexCount; 
    uint32_t instanceCount;
    uint32_t firstIndex;
    uint32_t vertexOffset;
    uint32_t firstInstance;

    RHICommandDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
    : indexCount(indexCount)
    , instanceCount(instanceCount)
    , firstIndex(firstIndex)
    , vertexOffset(vertexOffset)
    , firstInstance(firstInstance)
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandDrawIndirect : public RHICommand 
{
    RHIBufferRef argumentBuffer;
    uint32_t offset;
    uint32_t drawCount;

    RHICommandDrawIndirect(RHIBufferRef argumentBuffer, uint32_t offset, uint32_t drawCount)
    : argumentBuffer(argumentBuffer)
    , offset(offset)
    , drawCount(drawCount)
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

struct RHICommandDrawIndexedIndirect : public RHICommand 
{
    RHIBufferRef argumentBuffer;
    uint32_t offset;
    uint32_t drawCount;

    RHICommandDrawIndexedIndirect(RHIBufferRef argumentBuffer, uint32_t offset, uint32_t drawCount) 
    : argumentBuffer(argumentBuffer)
    , offset(offset)
    , drawCount(drawCount)
    {}

    virtual void Execute(RHICommandContextRef context) override final;
};

