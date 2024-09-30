#pragma once

#include "Function/Render/RHI/VulkanRHI/VulkanRHICache.h"
#include "VulkanUtil.h"
#include "VulkanRHIResource.h"
#include "Function/Render/RHI/RHI.h"
#include "Function/Render/RHI/RHIResource.h"
#include "Function/Render/RHI/RHIStructs.h"
#include "Platform/HAL/CriticalSection.h"

#include <volk.h>
#include <vma.h>
#include <array>
#include <cstdint>
#include <vector>

#define VULKAN_VERSION VK_API_VERSION_1_3

class VulkanRHIBackend : public RHIBackend
{
public:
    VulkanRHIBackend() = delete;

    VulkanRHIBackend(const RHIBackendInfo& info);

    virtual void Tick() override final;

    virtual void Destroy() override final;

    //ImGui ////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void InitImGui(GLFWwindow* window) override final;

    //基本资源 ////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual RHIQueueRef GetQueue(const RHIQueueInfo& info) override final;

    virtual RHISurfaceRef CreateSurface(GLFWwindow* window) override final;

    virtual RHISwapchainRef CreateSwapChain(const RHISwapchainInfo& info) override final;

    virtual RHICommandPoolRef CreateCommandPool(const RHICommandPoolInfo& info) override final;

    virtual RHICommandContextRef CreateCommandContext(RHICommandPoolRef pool) override final;

    //缓冲，纹理 ////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual RHIBufferRef CreateBuffer(const RHIBufferInfo& info) override final;

    virtual RHITextureRef CreateTexture(const RHITextureInfo& info) override final;

    virtual RHITextureViewRef CreateTextureView(const RHITextureViewInfo& info) override final;

    virtual RHISamplerRef CreateSampler(const RHISamplerInfo& info) override final;

    //着色器 ////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual RHIShaderRef CreateShader(const RHIShaderInfo& info) override final;

    virtual RHIRootSignatureRef CreateRootSignature(const RHIRootSignatureInfo& info) override final;

    //管线状态 ////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual RHIRenderPassRef CreateRenderPass(const RHIRenderPassInfo& info) override final;

    virtual RHIGraphicsPipelineRef CreateGraphicsPipeline(const RHIGraphicsPipelineInfo& info) override final;

    virtual RHIComputePipelineRef CreateComputePipeline(const RHIComputePipelineInfo& info) override final;

    virtual RHIRayTracingPipelineRef CreateRayTracingPipeline(const RHIRayTracingPipelineInfo& info) override final;

    //同步 ////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual RHIFenceRef CreateFence(bool signaled = false) override final;

    virtual RHISemaphoreRef CreateSemaphore() override final;

    //立即模式的命令接口 ////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual RHICommandListImmediateRef GetImmediateCommand() override final;




public:
    inline VkInstance GetInstance() const               { return instance; }
    inline VkPhysicalDevice GetPhysicalDevice() const   { return physicalDevice; }
    inline VkDevice GetLogicalDevice() const            { return logicalDevice; }
    inline VmaAllocator GetMemoryAllocator() const      { return memoryAllocator; }
    inline VkDescriptorPool GetDescriptorPool() const   { return descriptorPool; }

    VkRenderPass FindOrCreateVkRenderPass(const VulkanRenderPassAttachments& info) { return renderPassPool.Allocate(info).pass; }
    VkRenderPass CreateVkRenderPass(const VulkanRenderPassAttachments& info);

    VkFramebuffer FindOrCreateVkFramebuffer(const VkFramebufferCreateInfo& info) { return frameBufferPool.Allocate(info).frameBuffer; }
    VkFramebuffer CreateVkFramebuffer(const VkFramebufferCreateInfo& info);

private:

    // 实例
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    std::vector<VkLayerProperties> availableLayers;

    // 物理设备
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    VkPhysicalDeviceSamplerFilterMinmaxProperties filterMinmaxProperties;           //采样器特性
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties;   //光追特性
    std::vector<std::string> supportedExtensions;                                   //支持的全部扩展名

    // 逻辑设备
    VkDevice logicalDevice;

    // 队列
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    std::array<int32_t, QUEUE_TYPE_MAX_ENUM> queueIndices;
    std::array<std::array<RHIQueueRef, MAX_QUEUE_CNT>, QUEUE_TYPE_MAX_ENUM> queues; //预创建的所有队列

    // 内存分配
    VmaAllocator memoryAllocator;

    // 描述符池
    VkDescriptorPool descriptorPool;

    // 池化的renderPass和frameBuffer
    VkRenderPassCache renderPassPool;
    VkFramebufferCache frameBufferPool;

    // 立即模式命令队列
    RHICommandContextImmediateRef immediateCommandContext;
    RHICommandListImmediateRef immediateCommand;

    // 同步
    CriticalSectionRef sync;

    // 是否已初始化ImGui，用于析构时释放资源
    bool initImGui = false;

    void CreateInstance();
    void CreatePhysicalDevice();
    void CreateLogicalDevice();
    void CreateQueues();
    void CreateMemoryAllocator();
    void CreateDescriptorPool(); 
    void CreateImmediateCommand(); 

    friend class VulkanRHIRootSignature;    // 调用RegisterResource
};


class VulkanRHICommandContext : public RHICommandContext
{        
public:                
    VulkanRHICommandContext(RHICommandPoolRef pool, const VulkanRHIBackend& backend);

    virtual void Destroy() override final;

    virtual void BeginCommand() override final;

	virtual void EndCommand() override final;

    virtual void Execute(RHIFenceRef fence, RHISemaphoreRef waitSemaphore, RHISemaphoreRef signalSemaphore) override final;   

    virtual void TextureBarrier(const RHITextureBarrier& barrier) override final;

    virtual void BufferBarrier(const RHIBufferBarrier& barrier) override final;

    virtual void CopyTextureToBuffer(RHITextureRef src, TextureSubresourceLayers srcSubresource, RHIBufferRef dst, uint64_t dstOffset) override final;

    virtual void CopyBufferToTexture(RHIBufferRef src, uint64_t srcOffset, RHITextureRef dst, TextureSubresourceLayers dstSubresource) override final;

    virtual void CopyBuffer(RHIBufferRef src, uint64_t srcOffset, RHIBufferRef dst, uint64_t dstOffset, uint64_t size) override final;

    virtual void CopyTexture(RHITextureRef src, TextureSubresourceLayers srcSubresource, RHITextureRef dst, TextureSubresourceLayers dstSubresource) override final;

    virtual void GenerateMips(RHITextureRef src) override final;  

    virtual void PushEvent(const std::string& name, Color3 color) override final;   

	virtual void PopEvent() override final;

    virtual void BeginRenderPass(RHIRenderPassRef renderPass) override final;  

	virtual void EndRenderPass() override final;

    virtual void SetViewport(Offset2D min, Offset2D max) override final;

    virtual void SetScissor(Offset2D min, Offset2D max) override final;

    virtual void SetDepthBias(float constantBias, float slopeBias, float clampBias) override final;

    virtual void SetGraphicsPipeline(RHIGraphicsPipelineRef graphicsPipeline) override final;

    virtual void SetComputePipeline(RHIComputePipelineRef computePipeline) override final;	

    virtual void PushConstants(void* data, uint16_t size, ShaderFrequency frequency) override final;

    virtual void BindDescriptorSet(RHIDescriptorSetRef descriptor, uint32_t set) override final;

    virtual void BindVertexBuffer(RHIBufferRef vertexBuffer, uint32_t streamIndex, uint32_t offset) override final;

    virtual void BindIndexBuffer(RHIBufferRef indexBuffer, uint32_t offset) override final;

	virtual void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override final;

	virtual void DispatchIndirect(RHIBufferRef argumentBuffer, uint32_t argumentOffset) override final;

    virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override final;

    virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) override final;

    virtual void DrawIndirect(RHIBufferRef argumentBuffer, uint32_t offset, uint32_t drawCount) override final;

    virtual void DrawIndexedIndirect(RHIBufferRef argumentBuffer, uint32_t offset, uint32_t drawCount) override final;

    //ImGui /////////////////////////////////////////////////////////////////////////////////////

    virtual void ImGuiCreateFontsTexture() override final;

    virtual void ImGuiRenderDrawData() override final;

    const VkCommandBuffer& GetHandle() { return handle; }

private:
    VkCommandBuffer handle;
    VulkanRHICommandPool* pool;

    VulkanRHIRenderPass* renderPass;                // 运行时状态，随指令变化
    VulkanRHIGraphicsPipeline* graphicsPipeline;
    VulkanRHIComputePipeline* computePipeline;
    VulkanRHIRaytracingPipeline* rayTraycingPipeline;

    VkPipelineLayout GetCuttentPipelineLayout();
    VkPipelineBindPoint GetCuttentBindingPoint();
};

class VulkanRHICommandContextImmediate : public RHICommandContextImmediate 
{
public:
    VulkanRHICommandContextImmediate(VulkanRHIBackend& backend);

    virtual void Flush() override final;

    virtual void TextureBarrier(const RHITextureBarrier& barrier) override final;

    virtual void BufferBarrier(const RHIBufferBarrier& barrier) override final;

    virtual void CopyTextureToBuffer(RHITextureRef src, TextureSubresourceLayers srcSubresource, RHIBufferRef dst, uint64_t dstOffset) override final;

    virtual void CopyBufferToTexture(RHIBufferRef src, uint64_t srcOffset, RHITextureRef dst, TextureSubresourceLayers dstSubresource) override final;

    virtual void CopyBuffer(RHIBufferRef src, uint64_t srcOffset, RHIBufferRef dst, uint64_t dstOffset, uint64_t size) override final;

    virtual void CopyTexture(RHITextureRef src, TextureSubresourceLayers srcSubresource, RHITextureRef dst, TextureSubresourceLayers dstSubresource) override final;

    virtual void GenerateMips(RHITextureRef src) override final;  

private:
    VkCommandBuffer BeginSingleTimeCommand();
    void EndSingleTimeCommand(VkCommandBuffer commandBuffer);
    
    RHIFenceRef fence;
    RHIQueueRef queue;
    RHICommandPoolRef commandPool;

    VkCommandBuffer handle;
    VkDevice device;
};