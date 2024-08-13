#pragma once

#include "RHIResource.h"
#include "RHIStructs.h"

#include <GLFW/glfw3.h>
#include <array>
#include <cstdint>
#include <string>
#include <vector>

enum RHIBackendType
{
    BACKEND_VULKAN = 0,

    BACKEND_MAX_ENUM,    //
};

typedef struct RHIBackendInfo
{
    RHIBackendType type;

    bool enableDebug;
    bool enableRayTracing;

}RHIBackendInfo;

class RHIBackend    // DynamicRHI，主要做资源创建等与CommandList无关的工作   
{                   // FDynamicRHI是对没有上下文的操作的平台无关抽象（例如各种资源的创建等
private:
    static RHIBackendRef backend;

public:
    static RHIBackendRef Init(const RHIBackendInfo& info);

    static RHIBackendRef Get() { return backend; }

    virtual void Tick();    // 更新资源计数，清理无引用且长时间未使用资源

    virtual void Destroy();

    //基本资源 ////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual RHIQueueRef GetQueue(const RHIQueueInfo& info) = 0;

    virtual RHISurfaceRef CreateSurface(GLFWwindow* window) = 0;

    virtual RHISwapchainRef CreateSwapChain(const RHISwapchainInfo& info) = 0;

    virtual RHICommandPoolRef CreateCommandPool(const RHICommandPoolInfo& info) = 0;

    virtual RHICommandContextRef CreateCommandContext(RHICommandPool* pool) = 0;

    //缓冲，纹理 ////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual RHIBufferRef CreateBuffer(const RHIBufferInfo& info) = 0;

    virtual RHITextureRef CreateTexture(const RHITextureInfo& info) = 0;

    virtual RHITextureViewRef CreateTextureView(const RHITextureViewInfo& info) = 0;

    virtual RHISamplerRef CreateSampler(const RHISamplerInfo& info) = 0;

    //着色器 ////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual RHIShaderRef CreateShader(const RHIShaderInfo& info) = 0;

    virtual RHIRootSignatureRef CreateRootSignature(const RHIRootSignatureInfo& info) = 0;

    //管线状态 ////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual RHIRenderPassRef CreateRenderPass(const RHIRenderPassInfo& info) = 0;

    virtual RHIGraphicsPipelineRef CreateGraphicsPipeline(const RHIGraphicsPipelineInfo& info) = 0;

    virtual RHIComputePipelineRef CreateComputePipeline(const RHIComputePipelineInfo& info) = 0;

    virtual RHIRayTracingPipelineRef CreateRayTracingPipeline(const RHIRayTracingPipelineInfo& info) = 0;

    //同步 ////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual RHIFenceRef CreateFence(bool signaled) = 0;

    virtual RHISemaphoreRef CreateSemaphore() = 0;

    //立即模式的命令接口 ////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual RHIImmediateCommandRef GetImmediateCommand() = 0;

    
protected:
    RHIBackend() = delete;
    RHIBackend(const RHIBackendInfo& info) : backendInfo(info) {}

    void RegisterResource(RHIResourceRef resource) { resourceMap[resource->GetType()].push_back(resource); };     // 所有资源创建时应加入统一的资源管理

    std::array<std::vector<RHIResourceRef>, RHI_RESOURCE_TYPE_MAX_CNT> resourceMap;

    RHIBackendInfo backendInfo;
};


// IRHICommandContext是对带有上下文的操作的平台无关抽象（例如光栅管线，有一系列的固定管线设置，着色器绑定等），
// 由RHICommandList负责调用，由子类针对各api进行实现。实现上应该是存储了一整个静态的状态上下文，
// 每次调用会改这个上下文，在需要时才进行所有指令的生成（例如dispatch compute shader时），避免无意义的重复改变状态

// RHICommandContext，主要做命令录制的与CommandList相关的工作
// UE的FVulkanCommandListContext带了一个FVulkanCommandBufferManager，包含一个pool
// FVulkanDevice::InitGPU()中按线程数分配了一至多个CommandContext
// RHICommandList可以直接new出来，但是还得靠FRHICommandListBase::SwitchPipeline找一个绑定的上下文: GDynamicRHI->RHIGetCommandContext，
// 且在运行过程中保持对context的一对一的占用，直到FVulkanDynamicRHI::RHISubmitCommandLists才被释放（相当于只是给context做了个池化，实际上就是一对一）
// context的不少指令似乎只是在修改context里缓存的状态，等到draw call调用时将全部缓存的状态转换为状态设置指令(例如PendingGfxState->PrepareForDraw(Cmd);)，降低不必要的转换指令录制和执行？
class RHICommandContext : public RHIResource     
{
public:
    RHICommandContext() : RHIResource(RHI_COMMAND_CONTEXT) {}

    virtual void BeginCommand() = 0;

	virtual void EndCommand() = 0;  // 结束录制

    virtual void Execute(RHIFenceRef waitFence, RHISemaphoreRef waitSemaphore, RHISemaphoreRef signalSemaphore) = 0;     // 实际提交，如果延迟录制也该在对应线程调用该函数完成录制提交

    // UE RHI彻底做了资源状态（如VkImageLayout）等的屏蔽封装，代价是极其痛苦的RHI实现
    // 和BeginTransitions，FVulkanLayoutManager等有关
    // 参考Sakura Engine还是做暴露吧，与UE和解
    // resource state的屏蔽应该在RDG等层级处理，而不是RHI

    virtual void TextureBarrier(const RHITextureBarrier& barrier) = 0;

    virtual void BufferBarrier(const RHIBufferBarrier& barrier) = 0;

    virtual void CopyTextureToBuffer(RHITextureRef src, TextureSubresourceLayers srcSubresource, RHIBufferRef dst, uint64_t dstOffset) = 0;

    virtual void CopyBufferToTexture(RHIBufferRef src, uint64_t srcOffset, RHITextureRef dst, TextureSubresourceLayers dstSubresource) = 0;

    virtual void CopyBuffer(RHIBufferRef src, uint64_t srcOffset, RHIBufferRef dst, uint64_t dstOffset, uint64_t size) = 0;

    virtual void CopyTexture(RHITextureRef src, TextureSubresourceLayers srcSubresource, RHITextureRef dst, TextureSubresourceLayers dstSubresource) = 0;

    virtual void GenerateMips(RHITextureRef src) = 0;

    virtual void PushEvent(const std::string& name, Color3 color) = 0;   //Label?

	virtual void PopEvent() = 0;

    virtual void BeginRenderPass(RHIRenderPassRef renderPass) = 0;   //也可以运行时FindOrCreate相应的renderpass和framebuffer等，很多东西可以做中心化的查找表统一管理状态

	virtual void EndRenderPass() = 0;

    virtual void SetViewport(Offset2D min, Offset2D max) = 0;

    virtual void SetScissor(Offset2D min, Offset2D max) = 0;

    virtual void SetGraphicsPipeline(RHIGraphicsPipelineRef graphicsState) = 0;

    virtual void SetComputePipeline(RHIComputePipelineRef computeState) = 0;	

    virtual void PushConstants(void* data, uint16_t size, ShaderFrequency frequency) = 0;

    virtual void BindDescriptorSet(RHIDescriptorSetRef descriptor, uint32_t set) = 0;

    virtual void BindVertexBuffer(RHIBufferRef vertexBuffer, uint32_t streamIndex, uint32_t offset) = 0;

    virtual void BindIndexBuffer(RHIBufferRef indexBuffer, uint32_t offset) = 0;

	virtual void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;

	virtual void DispatchIndirect(RHIBufferRef argumentBuffer, uint32_t argumentOffset) = 0;

    virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;

    virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) = 0;

    virtual void DrawIndirect(RHIBufferRef argumentBuffer, uint32_t offset, uint32_t drawCount) = 0;

    virtual void DrawIndexedIndirect(RHIBufferRef argumentBuffer, uint32_t offset, uint32_t drawCount) = 0;

    // TODO 
    // virtual void BeginRenderQuery(RHIRenderQuery* RenderQuery) = 0;

	// virtual void EndRenderQuery(RHIRenderQuery* RenderQuery) = 0;

};

// 并不是所有command都需要在一个绘制的命令队列里完成，应该区别于RHICommandList单独开一个类？
// 例如生成mipmap，内存交换和屏障，
// vulkan可以简单的用VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT实现
// UE里对应了RHICommandListImmediate？？ UE的命令队列的功能太多了，也并不完全贴近VkCommandBuffer的抽象
class RHIImmediateCommand : public RHIResource 
{
public:
    RHIImmediateCommand() : RHIResource(RHI_IMMEDIATE_COMMAND) {}

    virtual void TextureBarrier(const RHITextureBarrier& barrier) = 0;

    virtual void BufferBarrier(const RHIBufferBarrier& barrier) = 0;

    virtual void CopyTextureToBuffer(RHITextureRef src, TextureSubresourceLayers srcSubresource, RHIBufferRef dst, uint64_t dstOffset) = 0;

    virtual void CopyBufferToTexture(RHIBufferRef src, uint64_t srcOffset, RHITextureRef dst, TextureSubresourceLayers dstSubresource) = 0;

    virtual void CopyBuffer(RHIBufferRef src, uint64_t srcOffset, RHIBufferRef dst, uint64_t dstOffset, uint64_t size) = 0;

    virtual void CopyTexture(RHITextureRef src, TextureSubresourceLayers srcSubresource, RHITextureRef dst, TextureSubresourceLayers dstSubresource) = 0;

    virtual void GenerateMips(RHITextureRef src) = 0;
};