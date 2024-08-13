#pragma once

#include "RHIStructs.h"
#include "RHICommandList.h"
#include "Platform/HAL/CriticalSection.h"
#include "Platform/HAL/PlatformProcess.h"

#include <cstdint>
#include <queue>
#include <vector>

//RHI资源 ////////////////////////////////////////////////////////////////////////////////////////////////////////

class RHIResource
{
public:
	RHIResource() = delete;
	RHIResource(RHIResourceType resourceType) : resourceType(resourceType) {};
	virtual ~RHIResource() {};

	inline RHIResourceType GetType() { return resourceType; }

private:
	RHIResourceType resourceType;
	uint32_t lastUseTick = 0;		// 最后一次使用的时间，帧单位

	virtual void Destroy() {};		// 资源销毁时调用，子类实现

	friend class RHIBackend;
};

//基本资源 ////////////////////////////////////////////////////////////////////////////////////////////////////////

class RHIQueue : public RHIResource
{
public:
	RHIQueue(const RHIQueueInfo& info) 
	: RHIResource(RHI_QUEUE)
	, info(info)
	{}

	virtual void WaitIdle() = 0;

protected:
	RHIQueueInfo info;
};

class RHISurface : public RHIResource
{
public:
	RHISurface() : RHIResource(RHI_SURFACE) {};

	inline Extent2D GetExetent() const { return extent; }

protected:
	Extent2D extent;
};

class RHISwapchain : public RHIResource
{
public:
	RHISwapchain(const RHISwapchainInfo& info) 
	: RHIResource(RHI_SWAPCHAIN)
	, info(info)
	{}

	virtual uint32_t GetCurrentFrameIndex() = 0;
	virtual RHITextureRef GetTexture(uint32_t index) = 0;
	virtual RHITextureRef GetNewFrame(RHIFenceRef fence, RHISemaphoreRef signalSemaphore) = 0;
	virtual void Present(RHISemaphoreRef waitSemaphore) = 0;

protected:
	RHISwapchainInfo info;
};

class RHICommandPool : public RHIResource
{
public:
	RHICommandPool(const RHICommandPoolInfo& info)
	: RHIResource(RHI_COMMAND_POOL)
	, info(info)
	, sync(PlatformProcess::CreateCriticalSection())
	{}

	RHICommandListRef CreateCommandList(bool byPass = true);
	
protected:
	RHICommandPoolInfo info;

	std::queue<RHICommandContextRef> idleContexts = {};  // 暂未运行的线程
	std::vector<RHICommandContextRef> contexts = {};     // 所有分配的线程
	CriticalSectionRef sync;

	void ReturnToPool(RHICommandContextRef commandContext) { idleContexts.push(commandContext); }
	friend class RHICommandList;
};

//缓冲，纹理 ////////////////////////////////////////////////////////////////////////////////////////////////////////

class RHIBuffer : public RHIResource
{
public:
	RHIBuffer(const RHIBufferInfo& info) 
	: RHIResource(RHI_BUFFER)
	, info(info) 
	{}

	virtual void* Map() = 0;
	virtual void UnMap() = 0;

	inline const RHIBufferInfo& GetInfo() const {return info; }

protected:
	RHIBufferInfo info;
};

class RHITextureView : public RHIResource
{
public:
	RHITextureView(const RHITextureViewInfo& info) 
	: RHIResource(RHI_TEXTURE_VIEW)
	, info(info)
	{}

	inline const RHITextureViewInfo& GetInfo() const {return info; }

protected:
	RHITextureViewInfo info;
};

class RHITexture : public RHIResource
{
public:
	RHITexture(const RHITextureInfo& info) 
	: RHIResource(RHI_TEXTURE)
	, info(info)
	{}

	Extent3D MipExtent(uint32_t mipLevel);

	inline const TextureSubresourceRange& GetDefaultSubresourceRange() const 	{ return defaultRange; }
	inline const TextureSubresourceLayers& GetDefaultSubresourceLayers() const 	{ return defaultLayers; }

	inline const RHITextureInfo& GetInfo() const {return info; }

protected:
	RHITextureInfo info;

	TextureSubresourceRange defaultRange = {};
	TextureSubresourceLayers defaultLayers = {};
};

class RHISampler : public RHIResource
{
public:
	RHISampler(const RHISamplerInfo& info) 
	: RHIResource(RHI_SAMPLER)
	, info(info)
	{}

	inline const RHISamplerInfo& GetInfo() const { return info; }

protected:
	RHISamplerInfo info;
};

//着色器 ////////////////////////////////////////////////////////////////////////////////////////////////////////

class RHIShader : public RHIResource
{
public:
	RHIShader(const RHIShaderInfo& info) 
	: RHIResource(RHI_SHADER)
	, info(info)
	{
		frequency = info.frequency;
	}

	ShaderFrequency GetFrequency() 				const { return frequency; }
	const ShaderReflectInfo& GetReflectInfo() 	const { return reflectInfo; }
	const RHIShaderInfo& GetInfo() 				const { return info; }

private:
	ShaderFrequency frequency;

protected:
	RHIShaderInfo info;
	ShaderReflectInfo reflectInfo;
};

class RHIRootSignature : public RHIResource	//对pipelinelayout, descriptorSetPool等的抽象
{
public:
	RHIRootSignature(const RHIRootSignatureInfo& info) 
	: RHIResource(RHI_ROOT_SIGNATURE)
	, info(info)
	{}

	virtual RHIDescriptorSetRef CreateDescriptorSet(uint32_t set) = 0;

	const RHIRootSignatureInfo& GetInfo() { return info; }

protected:
	RHIRootSignatureInfo info;
};

class RHIDescriptorSet : public RHIResource 
{
public:
	RHIDescriptorSet() : RHIResource(RHI_DESCRIPTOR_SET) {}

	virtual RHIDescriptorSet& UpdateDescriptor(const RHIDescriptorUpdateInfo& descriptorUpdateInfo) = 0;

	RHIDescriptorSet& UpdateDescriptors(const std::vector<RHIDescriptorUpdateInfo>& descriptorUpdateInfos) 
	{ 
		for(auto& info : descriptorUpdateInfos) UpdateDescriptor(info); 
		return *this;
	};
};

//管线状态 ////////////////////////////////////////////////////////////////////////////////////////////////////////

class RHIRenderPass : public RHIResource	// 在vulkan里相当于renderpass和framebuffer的整体抽象
{
public:
	RHIRenderPass(const RHIRenderPassInfo& info) 
	: RHIResource(RHI_RENDER_PASS) 
	, info(info)
	{}

	const RHIRenderPassInfo& GetInfo() { return info; }

protected:
	RHIRenderPassInfo info;
};

class RHIGraphicsPipeline : public RHIResource
{
public:
	RHIGraphicsPipeline(const RHIGraphicsPipelineInfo& info) 
	: RHIResource(RHI_GRAPHICS_PIPELINE_STATE) 
	, stateInfo(info)
	{}

	const RHIGraphicsPipelineInfo& GetInfo() { return stateInfo; }

protected:
	RHIGraphicsPipelineInfo stateInfo;
};

class RHIComputePipeline : public RHIResource
{
public:
	RHIComputePipeline(const RHIComputePipelineInfo& info) 
	: RHIResource(RHI_COMPUTE_PIPELINE_STATE)
	, stateInfo(info)
	{}

protected:
	RHIComputePipelineInfo stateInfo;
};

class RHIRayTracingPipeline : public RHIResource
{
public:
	RHIRayTracingPipeline(const RHIRayTracingPipelineInfo& info)
	: RHIResource(RHI_RAY_TRACING_PIPELINE_STATE) 
	, stateInfo(info)
	{}

protected:
	RHIRayTracingPipelineInfo stateInfo;
};

//同步 ////////////////////////////////////////////////////////////////////////////////////////////////////////

class RHIFence : public RHIResource
{
public:
	RHIFence()
	: RHIResource(RHI_FENCE) 
	{}

	virtual void Wait() = 0;
};

class RHISemaphore : public RHIResource
{
public:
	RHISemaphore()
	: RHIResource(RHI_SEMAPHORE) 
	{}
};

//TODO RenderQuery	StagingBuffer用于拷贝GPU到CPU




