#pragma once

#include "Function/Render/RHI/RHIResource.h"
#include "Function/Render/RHI/RHIStructs.h"

#include <volk.h>
#include <GLFW/glfw3.h>
#include <vma.h>
#include <cstdint>
#include <memory>
#include <vector>

class VulkanRHIBackend;

//基本资源 ////////////////////////////////////////////////////////////////////////////////////////////////////////

class VulkanRHIQueue : public RHIQueue
{
public:

	VulkanRHIQueue(const RHIQueueInfo& info, VkQueue queue, uint32_t queueFamilyIndex) 
	: RHIQueue(info)
	, handle(queue) 
	, queueFamilyIndex(queueFamilyIndex)
	{}

	virtual void WaitIdle() override final { vkQueueWaitIdle(handle); };

	const VkQueue& GetHandle() { return handle; }
	uint32_t GetQueueFamilyIndex() { return queueFamilyIndex; }

private:
	VkQueue handle;
	uint32_t queueFamilyIndex;
};

class VulkanRHISurface : public RHISurface
{
public:
	VulkanRHISurface(GLFWwindow* window, VulkanRHIBackend& backend);

	const VkSurfaceKHR& GetHandle() { return handle; }

	virtual void Destroy() override final;

private:
	VkSurfaceKHR handle;
};

class VulkanRHISwapchain : public RHISwapchain
{
public:
	VulkanRHISwapchain(const RHISwapchainInfo& info, VulkanRHIBackend& backend);

	virtual uint32_t GetCurrentFrameIndex() override final { return currentIndex; };
	virtual RHITextureRef GetTexture(uint32_t index) override final { return textures[index]; };
	virtual RHITextureRef GetNewFrame(RHIFenceRef fence, RHISemaphoreRef signalSemaphore) override final;
	virtual void Present(RHISemaphoreRef waitSemaphore) override final;

	const VkSwapchainKHR& GetHandle() { return handle; }

	virtual void Destroy() override final;

private:
	VkSwapchainKHR handle;
	VkSurfaceFormatKHR surfaceFormat;
	VkPresentModeKHR presentMode;

	std::vector<VkImage> images;
	VkFormat imageFormat;
	VkExtent2D imageExtent;

	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> availableFormats;
    std::vector<VkPresentModeKHR> availablePresentModes;

	std::vector<RHITextureRef> textures;
	uint32_t currentIndex;

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(VkFormat targetFormat);
    VkPresentModeKHR ChooseSwapPresentMode();
    VkExtent2D ChooseSwapExtent();
};

class VulkanRHICommandPool : public RHICommandPool
{
public:
	VulkanRHICommandPool(const RHICommandPoolInfo& info, VulkanRHIBackend& backend);

	RHIQueueRef GetQueue() { return info.queue; }

	const VkCommandPool& GetHandle() { return handle; }

	virtual void Destroy() override final;

private:
	VkCommandPool handle;
};

//缓冲，纹理 ////////////////////////////////////////////////////////////////////////////////////////////////////////

class VulkanRHIBuffer : public RHIBuffer
{
public:
	VulkanRHIBuffer(const RHIBufferInfo& info, VulkanRHIBackend& backend);

	const VkBuffer& GetHandle() { return handle; }

	virtual void* Map() override final;
	virtual void UnMap() override final;

	virtual void Destroy() override final;

private:
	VkBuffer handle;

	VmaAllocation allocation;
	VmaAllocationInfo allocationInfo;

	bool mapped = false;
	void* pointer = nullptr;
};

class VulkanRHITexture : public RHITexture
{
public:
	VulkanRHITexture(const RHITextureInfo& info, VulkanRHIBackend& backend, VkImage image = VK_NULL_HANDLE);

	const VkImage& GetHandle() { return handle; }

	virtual void Destroy() override final;

private:
	VkImage handle;

	VmaAllocation allocation;
	VmaAllocationInfo allocationInfo;
};

class VulkanRHITextureView : public RHITextureView
{
public:
	VulkanRHITextureView(const RHITextureViewInfo& info, VulkanRHIBackend& backend);

	const VkImageView& GetHandle() { return handle; }

	virtual void Destroy() override final;

private:
	VkImageView handle;
};

class VulkanRHISampler : public RHISampler
{
public:
	VulkanRHISampler(const RHISamplerInfo& info, VulkanRHIBackend& backend);

	const VkSampler& GetHandle() { return handle; }

	virtual void Destroy() override final;

private:
	VkSampler handle;
};

//着色器 ////////////////////////////////////////////////////////////////////////////////////////////////////////

class VulkanRHIShader : public RHIShader
{
public:
	VulkanRHIShader(const RHIShaderInfo& info, VulkanRHIBackend& backend);

	VkPipelineShaderStageCreateInfo GetShaderStageCreateInfo();

	const VkShaderModule& GetHandle() { return handle; }

	virtual void Destroy() override final;

private:
	VkShaderModule handle;
};

class VulkanRHIRootSignature : public RHIRootSignature
{
public:
	struct SetInfo
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		VkDescriptorSetLayout layout;
	};

	VulkanRHIRootSignature(const RHIRootSignatureInfo& info, VulkanRHIBackend& backend);

	virtual RHIDescriptorSetRef CreateDescriptorSet(uint32_t set) override final;

	const std::vector<SetInfo>& GetSetInfos() { return setInfos; }

	virtual void Destroy() override final;

private:
	std::vector<SetInfo> setInfos;
};

class VulkanRHIDescriptorSet : public RHIDescriptorSet
{
public:
	VulkanRHIDescriptorSet(VkDescriptorSetLayout setLayout, VulkanRHIBackend& backend);

	virtual RHIDescriptorSet& UpdateDescriptor(const RHIDescriptorUpdateInfo& descriptorUpdateInfo) override final;

	const VkDescriptorSet& GetHandle() { return handle; }

	virtual void Destroy() override final;

private:
	VkDescriptorSet handle;
};

//管线状态 ////////////////////////////////////////////////////////////////////////////////////////////////////////

class VulkanRHIRenderPass : public RHIRenderPass
{
public:
	VulkanRHIRenderPass(const RHIRenderPassInfo& info, VulkanRHIBackend& backend);

	const VkRenderPass& GetHandle() { return handle; }
	const VkFramebuffer& GetFrameBuffer() { return frameBuffer; }

	virtual void Destroy() override final;

private:
	VkRenderPass handle;
	VkFramebuffer frameBuffer;
};

class VulkanRHIGraphicsPipeline : public RHIGraphicsPipeline
{
public:
	VulkanRHIGraphicsPipeline(const RHIGraphicsPipelineInfo& info, VulkanRHIBackend& backend);

	VkPipelineLayout GetPipelineLayout() { return pipelineLayout; }

	const VkPipeline& GetHandle() { return handle; }

	void Bind(VkCommandBuffer commandBuffer);

	virtual void Destroy() override final;

private:
	VkPipeline handle;
	VkPipelineLayout pipelineLayout;

	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	std::vector<VkPipelineColorBlendAttachmentState> blendStates;

	// 默认开启的动态设置状态
	std::vector<VkDynamicState> dynamicStates = 
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_LINE_WIDTH,
		VK_DYNAMIC_STATE_VERTEX_INPUT_EXT,
		VK_DYNAMIC_STATE_DEPTH_BIAS,
        // VK_DYNAMIC_STATE_BLEND_CONSTANTS,
        // VK_DYNAMIC_STATE_DEPTH_BOUNDS,
        // VK_DYNAMIC_STATE_STENCIL_REFERENCE
    };

	VkPipelineVertexInputStateCreateInfo GetInputStateCreateInfo(const VertexInputStateInfo& vertexInputState);
    VkPipelineInputAssemblyStateCreateInfo GetPipelineInputAssemblyStateCreateInfo(const PrimitiveType& primitiveType);
    VkPipelineViewportStateCreateInfo GetPipelineViewportStateCreateInfo();
    VkPipelineRasterizationStateCreateInfo GetPipelineRasterizationStateCreateInfo(const RHIRasterizerStateInfo& rasterizerState);
    VkPipelineMultisampleStateCreateInfo GetPipelineMultisampleStateCreateInfo();
    VkPipelineColorBlendStateCreateInfo GetPipelineColorBlendStateCreateInfo(const RHIBlendStateInfo& blendState, uint32_t size);
    VkPipelineDepthStencilStateCreateInfo GetPipelineDepthStencilStateCreateInfo(const RHIDepthStencilStateInfo& depthStencilState);
	VkPipelineDynamicStateCreateInfo GetPipelineDynamicStateCreateInfo();

	void GetDynamicInputStateCreateInfo(const VertexInputStateInfo& vertexInputState);
	std::vector<VkVertexInputAttributeDescription2EXT> dynamicAttributeDescriptions;
    std::vector<VkVertexInputBindingDescription2EXT> dynamicBindingDescriptions;
};

class VulkanRHIComputePipeline : public RHIComputePipeline
{
public:
	VulkanRHIComputePipeline(const RHIComputePipelineInfo& info, VulkanRHIBackend& backend);

	VkPipelineLayout GetPipelineLayout() { return pipelineLayout; }

	const VkPipeline& GetHandle() { return handle; }

	void Bind(VkCommandBuffer commandBuffer);

	virtual void Destroy() override final;

private:
	VkPipeline handle;
	VkPipelineLayout pipelineLayout;
};

class VulkanRHIRaytracingPipeline : public RHIRayTracingPipeline
{

};

//同步 ////////////////////////////////////////////////////////////////////////////////////////////////////////

class VulkanRHIFence : public RHIFence
{
public:
	VulkanRHIFence(bool signaled, VulkanRHIBackend& backend);

	virtual void Wait() override final;

	const VkFence& GetHandle() { return handle; }

	virtual void Destroy() override final;

private:
	VkFence handle;
};

class VulkanRHISemaphore : public RHISemaphore
{
public:
	VulkanRHISemaphore(VulkanRHIBackend& backend);

	const VkSemaphore& GetHandle() { return handle; }

	virtual void Destroy() override final;

private:
	VkSemaphore handle;
};







template<class T>
struct VulkanResourceTraits
{};

template<>
struct VulkanResourceTraits<RHISurface>
{
	typedef VulkanRHISurface ConcreteType;
	typedef std::shared_ptr<VulkanRHISurface> ConcretePointerType;
};

template<>
struct VulkanResourceTraits<RHIQueue>
{
	typedef VulkanRHIQueue ConcreteType;
	typedef std::shared_ptr<VulkanRHIQueue> ConcretePointerType;
};

template<>
struct VulkanResourceTraits<RHICommandPool>
{
	typedef VulkanRHICommandPool ConcreteType;
	typedef std::shared_ptr<VulkanRHICommandPool> ConcretePointerType;
};

template<>
struct VulkanResourceTraits<RHIRenderPass>
{
	typedef VulkanRHIRenderPass ConcreteType;
	typedef std::shared_ptr<VulkanRHIRenderPass> ConcretePointerType;
};

template<>
struct VulkanResourceTraits<RHIBuffer>
{
	typedef VulkanRHIBuffer ConcreteType;
	typedef std::shared_ptr<VulkanRHIBuffer> ConcretePointerType;
};

template<>
struct VulkanResourceTraits<RHITexture>
{
	typedef VulkanRHITexture ConcreteType;
	typedef std::shared_ptr<VulkanRHITexture> ConcretePointerType;
};

template<>
struct VulkanResourceTraits<RHITextureView>
{
	typedef VulkanRHITextureView ConcreteType;
	typedef std::shared_ptr<VulkanRHITextureView> ConcretePointerType;
};

template<>
struct VulkanResourceTraits<RHISampler>
{
	typedef VulkanRHISampler ConcreteType;
	typedef std::shared_ptr<VulkanRHISampler> ConcretePointerType;
};

template<>
struct VulkanResourceTraits<RHIShader>
{
	typedef VulkanRHIShader ConcreteType;
	typedef std::shared_ptr<VulkanRHIShader> ConcretePointerType;
};

template<>
struct VulkanResourceTraits<RHIRootSignature>
{
	typedef VulkanRHIRootSignature ConcreteType;
	typedef std::shared_ptr<VulkanRHIRootSignature> ConcretePointerType;
};

template<>
struct VulkanResourceTraits<RHIDescriptorSet>
{
	typedef VulkanRHIDescriptorSet ConcreteType;
	typedef std::shared_ptr<VulkanRHIDescriptorSet> ConcretePointerType;
};

template<>
struct VulkanResourceTraits<RHIGraphicsPipeline>
{
	typedef VulkanRHIGraphicsPipeline ConcreteType;
	typedef std::shared_ptr<VulkanRHIGraphicsPipeline> ConcretePointerType;
};

template<>
struct VulkanResourceTraits<RHIComputePipeline>
{
	typedef VulkanRHIComputePipeline ConcreteType;
	typedef std::shared_ptr<VulkanRHIComputePipeline> ConcretePointerType;
};

template<>
struct VulkanResourceTraits<RHIFence>
{
	typedef VulkanRHIFence ConcreteType;
	typedef std::shared_ptr<VulkanRHIFence> ConcretePointerType;
};

template<>
struct VulkanResourceTraits<RHISemaphore>
{
	typedef VulkanRHISemaphore ConcreteType;
	typedef std::shared_ptr<VulkanRHISemaphore> ConcretePointerType;
};


// 类型萃取获得子类
template<typename RHIType>
static inline typename VulkanResourceTraits<RHIType>::ConcreteType* ResourceCast(RHIType* resource)
{
	return static_cast<typename VulkanResourceTraits<RHIType>::ConcreteType*>(resource);
}

template<typename RHIType>
static inline typename VulkanResourceTraits<RHIType>::ConcretePointerType ResourceCast(std::shared_ptr<RHIType> resource)
{
	return static_pointer_cast<typename VulkanResourceTraits<RHIType>::ConcreteType>(resource);
}