#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#define MAX_QUEUE_CNT 2			//每个队列族的最大队列数目
#define MAX_RENDER_TARGETS 8	//允许同时绑定的最大RT数目
#define MAX_DESCRIPTOR_SETS 8	//允许绑定的最大描述符集数目

typedef std::shared_ptr<class RHICommandContext> RHICommandContextRef;
typedef std::shared_ptr<class RHIBackend> RHIBackendRef;
typedef std::shared_ptr<class RHIImmediateCommand> RHIImmediateCommandRef;
typedef std::shared_ptr<class RHIResource> RHIResourceRef;
typedef std::shared_ptr<class RHIBuffer> RHIBufferRef;
typedef std::shared_ptr<class RHITexture> RHITextureRef;
typedef std::shared_ptr<class RHITextureView> RHITextureViewRef;
typedef std::shared_ptr<class RHISampler> RHISamplerRef;
typedef std::shared_ptr<class RHIShader> RHIShaderRef;
typedef std::shared_ptr<class RHIRootSignature> RHIRootSignatureRef;
typedef std::shared_ptr<class RHIDescriptorSet> RHIDescriptorSetRef;
typedef std::shared_ptr<class RHIRenderPass> RHIRenderPassRef;
typedef std::shared_ptr<class RHIGraphicsPipeline> RHIGraphicsPipelineRef;
typedef std::shared_ptr<class RHIComputePipeline> RHIComputePipelineRef;
typedef std::shared_ptr<class RHIRayTracingPipeline> RHIRayTracingPipelineRef;
typedef std::shared_ptr<class RHIQueue> RHIQueueRef;
typedef std::shared_ptr<class RHISurface> RHISurfaceRef;
typedef std::shared_ptr<class RHISwapchain> RHISwapchainRef;
typedef std::shared_ptr<class RHICommandPool> RHICommandPoolRef;
typedef std::shared_ptr<class RHIFence> RHIFenceRef;
typedef std::shared_ptr<class RHISemaphore> RHISemaphoreRef;

enum RHIResourceType	// 此处的倒序也是有效的析构顺序
{
	RHI_BUFFER = 0,
	RHI_TEXTURE,
	RHI_TEXTURE_VIEW,
	RHI_SAMPLER,

	RHI_SHADER,
	RHI_ROOT_SIGNATURE,
	RHI_DESCRIPTOR_SET,
	
	RHI_RENDER_PASS,
	RHI_GRAPHICS_PIPELINE_STATE,
	RHI_COMPUTE_PIPELINE_STATE,
	RHI_RAY_TRACING_PIPELINE_STATE,

	RHI_QUEUE,
	RHI_SURFACE,
	RHI_SWAPCHAIN,
	RHI_COMMAND_POOL,
	RHI_COMMAND_CONTEXT,
	RHI_IMMEDIATE_COMMAND,
	RHI_FENCE,
	RHI_SEMAPHORE,

	RHI_RESOURCE_TYPE_MAX_CNT,	//
};

enum QueueType
{
	QUEUE_TYPE_GRAPHICS = 0,
	QUEUE_TYPE_COMPUTE,
	QUEUE_TYPE_TRANSFER,

	QUEUE_TYPE_MAX_ENUM,	//
};

enum MemoryUsage
{
    MEMORY_USAGE_UNKNOWN = 0,
    MEMORY_USAGE_GPU_ONLY = 1,		// 仅GPU使用，在VRAM显存上分配，不可绑定
    MEMORY_USAGE_CPU_ONLY = 2,		// HOST_VISIBLE &&  HOST_COHERENT 及时同步，不需要flush到GPU，GPU可访问但是很慢
    MEMORY_USAGE_CPU_TO_GPU = 3,	// HOST_VISIBLE CPU端uncached，用于CPU端频繁进行数据写入，GPU端对数据进行读取
    MEMORY_USAGE_GPU_TO_CPU = 4,	// HOST_VISIBLE CPU端cached，用于被GPU写入且被CPU读取

    MEMORY_USAGE_MAX_ENUM = 0x7FFFFFFF,		//
};

enum ResourceTypeBits	//资源类型，封装了UsageFlag和DescriptorType，在底层实现做推断
{
    RESOURCE_TYPE_NONE = 0x00000000,
    RESOURCE_TYPE_SAMPLER = 0x00000001,
    RESOURCE_TYPE_TEXTURE = 0x00000002,
	RESOURCE_TYPE_RW_TEXTURE = 0x00000004,
	RESOURCE_TYPE_TEXTURE_CUBE = 0x00000008,
	RESOURCE_TYPE_RENDER_TARGET = 0x00000010,
	RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER = 0x00000020,
    RESOURCE_TYPE_BUFFER = 0x00000040,
    RESOURCE_TYPE_RW_BUFFER = 0x00000080,
    RESOURCE_TYPE_UNIFORM_BUFFER = 0x00000100,
    RESOURCE_TYPE_VERTEX_BUFFER = 0x00000200,
    RESOURCE_TYPE_INDEX_BUFFER = 0x00000400,
    RESOURCE_TYPE_INDIRECT_BUFFER = 0x00000800,
    RESOURCE_TYPE_TEXEL_BUFFER = 0x00001000,
    RESOURCE_TYPE_RW_TEXEL_BUFFER = 0x00002000,
    RESOURCE_TYPE_RAY_TRACING = 0x00004000,

    RESOURCE_TYPE_MAX_ENUM = 0x7FFFFFFF,	//
};
typedef uint32_t ResourceType;

enum BufferCreationFlagBits
{
	BUFFER_CREATION_NONE = 0x00000000,
	BUFFER_CREATION_PERSISTENT_MAP = 0x00000001,

	BUFFER_CREATION_MAX_ENUM = 0x7FFFFFFF,	//
};
typedef uint32_t BufferCreationFlags;

enum TextureCreationFlagBits
{
	TEXTURE_CREATION_NONE = 0x00000000,
	TEXTURE_CREATION_FORCE_2D = 0x00000001,
	TEXTURE_CREATION_FORCE_3D = 0x00000002,

	TEXTURE_CREATION_MAX_ENUM = 0x7FFFFFFF,	//
};
typedef uint32_t TextureCreationFlags;

enum RHIResourceState	//根据状态来设置对应的barrier做转换以及同步，buffer也有		
{
    RESOURCE_STATE_UNDEFINED = 0,
	RESOURCE_STATE_COMMON,
	RESOURCE_STATE_TRANSFER_SRC,
    RESOURCE_STATE_TRANSFER_DST,
    RESOURCE_STATE_VERTEX_BUFFER,
    RESOURCE_STATE_INDEX_BUFFER,
    RESOURCE_STATE_COLOR_ATTACHMENT,
	RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT,
    RESOURCE_STATE_UNORDERED_ACCESS,
	RESOURCE_STATE_SHADER_RESOURCE,
    RESOURCE_STATE_INDIRECT_ARGUMENT,
    RESOURCE_STATE_PRESENT,
    RESOURCE_STATE_ACCELERATION_STRUCTURE,

    RESOURCE_STATE_MAX_ENUM,	//
};

enum TextureFormat 
{
	FORMAT_UKNOWN = 0,

	FORMAT_R8_SRGB,
	FORMAT_R8G8_SRGB,
	FORMAT_R8G8B8_SRGB,
	FORMAT_R8G8B8A8_SRGB,

	FORMAT_R16_SFLOAT,
	FORMAT_R16G16_SFLOAT,
	FORMAT_R16G16B16_SFLOAT,
	FORMAT_R16G16B16A16_SFLOAT,
	FORMAT_R32_SFLOAT,
	FORMAT_R32G32_SFLOAT,
	FORMAT_R32G32B32_SFLOAT,
	FORMAT_R32G32B32A32_SFLOAT,

	FORMAT_R8_UNORM,
	FORMAT_R8G8_UNORM,
	FORMAT_R8G8B8_UNORM,
	FORMAT_R8G8B8A8_UNORM,
	FORMAT_R16_UNORM,
	FORMAT_R16G16_UNORM,
	FORMAT_R16G16B16_UNORM,
	FORMAT_R16G16B16A16_UNORM,

	FORMAT_R8_UINT,
	FORMAT_R8G8_UINT,
	FORMAT_R8G8B8_UINT,
	FORMAT_R8G8B8A8_UINT,
	FORMAT_R16_UINT,
	FORMAT_R16G16_UINT,
	FORMAT_R16G16B16_UINT,
	FORMAT_R16G16B16A16_UINT,
	FORMAT_R32_UINT,
	FORMAT_R32G32_UINT,
	FORMAT_R32G32B32_UINT,
	FORMAT_R32G32B32A32_UINT,

	FORMAT_R8_SINT,
	FORMAT_R8G8_SINT,
	FORMAT_R8G8B8_SINT,
	FORMAT_R8G8B8A8_SINT,
	FORMAT_R16_SINT,
	FORMAT_R16G16_SINT,
	FORMAT_R16G16B16_SINT,
	FORMAT_R16G16B16A16_SINT,
	FORMAT_R32_SINT,
	FORMAT_R32G32_SINT,
	FORMAT_R32G32B32_SINT,
	FORMAT_R32G32B32A32_SINT,

	FORMAT_D32_SFLOAT, 
	FORMAT_D32_SFLOAT_S8_UINT, 
	FORMAT_D24_UNORM_S8_UINT,

	FORMAT_MAX_ENUM, 	//
};

static bool IsDepthStencilFormat(TextureFormat format)
{
	switch (format) {
	case FORMAT_D32_SFLOAT:
	case FORMAT_D32_SFLOAT_S8_UINT:
	case FORMAT_D24_UNORM_S8_UINT:
		return true;
	default:
		return false;
	}
}

static bool IsStencilFormat(TextureFormat format)
{
	switch (format) {
	case FORMAT_D32_SFLOAT_S8_UINT:
	case FORMAT_D24_UNORM_S8_UINT:
		return true;
	default:
		return false;
	}
}

enum FilterType 
{
    FILTER_TYPE_NEAREST = 0,
    FILTER_TYPE_LINEAR,

    FILTER_TYPE_MAX_ENUM,	//
};

enum MipMapMode
{
    MIPMAP_MODE_NEAREST = 0,
    MIPMAP_MODE_LINEAR,

    MIPMAP_MODE_MAX_ENUM_BIT,	//
};

enum AddressMode
{
    ADDRESS_MODE_MIRROR,
    ADDRESS_MODE_REPEAT,
    ADDRESS_MODE_CLAMP_TO_EDGE,
    ADDRESS_MODE_CLAMP_TO_BORDER,

    ADDRESS_MODE_MAX_ENUM,	//
};

enum TextureViewType
{
	VIEW_TYPE_UNDEFINED = 0,
	VIEW_TYPE_1D,
    VIEW_TYPE_2D,
    VIEW_TYPE_3D,
    VIEW_TYPE_CUBE,
    VIEW_TYPE_1D_ARRAY,
    VIEW_TYPE_2D_ARRAY,
    VIEW_TYPE_CUBE_ARRAY,

    VIEW_TYPE_MAX_ENUM,		//	
};

enum TextureAspectFlagBits
{
	TEXTURE_ASPECT_NONE = 0x00000000,
	TEXTURE_ASPECT_COLOR = 0x00000001,
	TEXTURE_ASPECT_DEPTH = 0x00000002,
	TEXTURE_ASPECT_STENCIL = 0x00000004,

	TEXTURE_ASPECT_DEPTH_STENCIL = TEXTURE_ASPECT_DEPTH | TEXTURE_ASPECT_STENCIL,

	TEXTURE_ASPECT_MAX_ENUM,	//
};
typedef uint32_t TextureAspectFlags;

enum ShaderFrequencyBits
{
	SHADER_FREQUENCY_COMPUTE = 0x00000001,
	SHADER_FREQUENCY_VERTEX = 0x00000002,
	SHADER_FREQUENCY_FRAGMENT = 0x00000004,
	SHADER_FREQUENCY_GEOMETRY = 0x00000008,
	SHADER_FREQUENCY_RAY_GEN = 0x00000010,
	SHADER_FREQUENCY_CLOSEST_HIT = 0x00000020,
	SHADER_FREQUENCY_RAY_MISS = 0x00000040,
	SHADER_FREQUENCY_INTERSECTION = 0x00000080,
	SHADER_FREQUENCY_ANY_HIT = 0x00000100,
	SHADER_FREQUENCY_MESH = 0x00000200,

	SHADER_FREQUENCY_GRAPHICS = 	SHADER_FREQUENCY_VERTEX | 
									SHADER_FREQUENCY_FRAGMENT | 
									SHADER_FREQUENCY_GEOMETRY | 
									SHADER_FREQUENCY_MESH,	
	SHADER_FREQUENCY_RAY_TRACING = 	SHADER_FREQUENCY_RAY_GEN | 
									SHADER_FREQUENCY_CLOSEST_HIT | 
									SHADER_FREQUENCY_RAY_MISS | 
									SHADER_FREQUENCY_INTERSECTION | 
									SHADER_FREQUENCY_ANY_HIT,
	SHADER_FREQUENCY_ALL =			SHADER_FREQUENCY_GRAPHICS | 
									SHADER_FREQUENCY_COMPUTE | 	
									SHADER_FREQUENCY_RAY_TRACING,

	SHADER_FREQUENCY_MAX_ENUM = 0x7FFFFFFF, //
};
typedef uint32_t ShaderFrequency;

enum AttachmentLoadOp 
{
    ATTACHMENT_LOAD_OP_LOAD = 0,
    ATTACHMENT_LOAD_OP_CLEAR,
    ATTACHMENT_LOAD_OP_DONT_CARE,

    ATTACHMENT_LOAD_OP_MAX_ENUM,	//
};

enum AttachmentStoreOp 
{
    ATTACHMENT_STORE_OP_STORE = 0,
    ATTACHMENT_STORE_OP_DONT_CARE = 1,

    ATTACHMENT_STORE_OP_MAX_ENUM, 	//
} ;

enum PrimitiveType
{
	PRIMITIVE_TYPE_TRIANGLE_LIST = 0,
	PRIMITIVE_TYPE_TRIANGLE_STRIP,
	PRIMITIVE_TYPE_LINE_LIST,
	PRIMITIVE_TYPE_POINT_LIST,	

	PRIMITIVE_TYPE_MAX_ENUM,	//
};

enum RasterizerFillMode
{
	FILL_MODE_POINT = 0,
	FILL_MODE_WIREFRAME,
	FILL_MODE_SOLID,

    FILL_MODE_MAX_ENUM,  //
};

enum RasterizerCullMode
{
	CULL_MODE_NONE = 0,
	CULL_MODE_FRONT,     
	CULL_MODE_BACK,

    CULL_MODE_MAX_ENUM,  //
};

enum RasterizerDepthClipMode
{
	DEPTH_CLIP = 0,
	DEPTH_CLAMP,

	DEPTH_CLIP_MODE_MAX_ENUM,    //
};

enum CompareFunction
{
	COMPARE_FUNCTION_LESS = 0,
	COMPARE_FUNCTION_LESS_EQUAL,
	COMPARE_FUNCTION_GREATER,
	COMPARE_FUNCTION_GREATER_EQUAL,
	COMPARE_FUNCTION_EQUAL,
	COMPARE_FUNCTION_NOT_EQUAL,
	COMPARE_FUNCTION_NEVER,
	COMPARE_FUNCTION_ALWAYS,

	COMPARE_FUNCTION_MAX_ENUM,   //
};

enum StencilOp
{
    STENCIL_OP_KEEP = 0,
    STENCIL_OP_ZERO,
    STENCIL_OP_REPLACE,
    STENCIL_OP_SATURATED_INCREMENT,
    STENCIL_OP_SATURATED_DECREMENT,
    STENCIL_OP_INVERT,
    STENCIL_OP_INCREMENT,
    STENCIL_OP_DECREMENT,

    STENCIL_OP_MAX_ENUM, //
};

enum BlendOp
{
	BLEND_OP_ADD = 0,
    BLEND_OP_SUBTRACT,
	BLEND_OP_REVERSE_SUBTRACT,
    BLEND_OP_MIN,
    BLEND_OP_MAX,

    BLEND_OP_MAX_ENUM, //
};

enum BlendFactor
{
    BLEND_FACTOR_ZERO = 0,
    BLEND_FACTOR_ONE,
    BLEND_FACTOR_SRC_COLOR,
    BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
    BLEND_FACTOR_DST_COLOR,
    BLEND_FACTOR_ONE_MINUS_DST_COLOR,
    BLEND_FACTOR_SRC_ALPHA,
    BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    BLEND_FACTOR_DST_ALPHA,
    BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
    BLEND_FACTOR_SRC_ALPHA_SATURATE,
    BLEND_FACTOR_CONSTANT_COLOR,
    BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,

    BLEND_FACTOR_MAX_ENUM, //
};

enum ColorWriteMaskBits
{
	COLOR_MASK_RED   = 0x01,
	COLOR_MASK_GREEN = 0x02,
	COLOR_MASK_BLUE  = 0x04,
	COLOR_MASK_ALPHA = 0x08,

	COLOR_MASK_NONE  = 0,
	COLOR_MASK_RGB   = COLOR_MASK_RED | COLOR_MASK_GREEN | COLOR_MASK_BLUE,
	COLOR_MASK_RGBA  = COLOR_MASK_RED | COLOR_MASK_GREEN | COLOR_MASK_BLUE | COLOR_MASK_ALPHA,
	COLOR_MASK_RG    = COLOR_MASK_RED | COLOR_MASK_GREEN,
	COLOR_MASK_BA    = COLOR_MASK_BLUE | COLOR_MASK_ALPHA,
};
typedef uint32_t ColorWriteMasks;

typedef struct RHIIndexedIndirectCommand 
{
    uint32_t    indexCount;
    uint32_t    instanceCount;
    uint32_t    firstIndex;
    int32_t     vertexOffset;
    uint32_t    firstInstance;
} RHIIndexedIndirectCommand;

typedef struct RHIIndirectCommand 
{
    uint32_t    vertexCount;
    uint32_t    instanceCount;
    uint32_t    firstVertex;
    uint32_t    firstInstance;
} RHIIndirectCommand;

typedef struct Extent2D 
{
    uint32_t    width;
    uint32_t    height;

	friend bool operator==(const Extent2D& a, const Extent2D& b)
	{
		return 	a.width == b.width &&
				a.height == b.height;
	}

} Extent2D;

typedef struct Extent3D 
{
    uint32_t    width;
    uint32_t    height;
    uint32_t    depth;

	friend bool operator==(const Extent3D& a, const Extent3D& b)
	{
		return 	a.width == b.width &&
				a.height == b.height &&
				a.depth == b.depth;
	}

} Extent3D;

typedef struct Offset2D 
{
    int32_t    x;
    int32_t    y;

	friend bool operator==(const Offset2D& a, const Offset2D& b)
	{
		return 	a.x == b.x &&
				a.y == b.y;
	}

} Offset2D;

typedef struct Offset3D 
{
    int32_t    x;
    int32_t    y;
    int32_t    z;

	friend bool operator==(const Offset3D& a, const Offset3D& b)
	{
		return 	a.x == b.x &&
				a.y == b.y &&
				a.z == b.z;
	}

} Offset3D;

typedef struct Rect2D 
{
    Offset2D    offset;
    Extent2D    extent;
} Rect2D;

typedef struct Color3 {
    float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
} Color3;

typedef struct Color4 
{
    float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 0.0f;
} Color4;

typedef struct TextureSubresourceRange 
{
	TextureAspectFlags	  aspect = TEXTURE_ASPECT_NONE;
    uint32_t              baseMipLevel = 0;
    uint32_t              levelCount = 0;
    uint32_t              baseArrayLayer = 0;
    uint32_t              layerCount = 0;

	friend bool operator==(const TextureSubresourceRange& a, const TextureSubresourceRange& b)
	{
		return 	a.aspect == b.aspect &&
				a.baseMipLevel == b.baseMipLevel &&
				a.levelCount == b.levelCount &&
				a.baseArrayLayer == b.baseArrayLayer &&
				a.layerCount == b.layerCount;
	}

} TextureSubresourceRange;

typedef struct TextureSubresourceLayers
{
	TextureAspectFlags	  aspect = TEXTURE_ASPECT_NONE;
    uint32_t              mipLevel = 0;
    uint32_t              baseArrayLayer = 0;
    uint32_t              layerCount = 0;

	friend bool operator==(const TextureSubresourceLayers& a, const TextureSubresourceLayers& b)
	{
		return 	a.aspect == b.aspect &&
				a.mipLevel == b.mipLevel &&
				a.baseArrayLayer == b.baseArrayLayer &&
				a.layerCount == b.layerCount;
	}

} TextureSubresourceLayers;

typedef struct RHIQueueInfo
{
	QueueType type;
	uint32_t index;

} RHIQueueInfo;

typedef struct RHISwapchainInfo
{
	RHISurfaceRef surface;
	RHIQueueRef presentQueue;

	uint32_t imageCount;
	Extent2D extent;
	TextureFormat format;

} RHISwapchainInfo;

typedef struct RHICommandPoolInfo
{
	RHIQueueRef queue;

} RHICommandPoolInfo;

typedef struct RHIBufferInfo
{
	uint64_t size;

	MemoryUsage memoryUsage = MEMORY_USAGE_GPU_ONLY;
	ResourceType type = RESOURCE_TYPE_BUFFER;

	BufferCreationFlags creationFlag = BUFFER_CREATION_NONE;

} RHIBufferInfo;

typedef struct RHITextureInfo 
{
	TextureFormat format;
	Extent3D extent;
	uint32_t arrayLayers = 1;
	uint32_t mipLevels = 1;

	MemoryUsage memoryUsage = MEMORY_USAGE_GPU_ONLY;
	ResourceType type = RESOURCE_TYPE_TEXTURE;

	TextureCreationFlags creationFlag = TEXTURE_CREATION_NONE;

} RHITextureInfo;

typedef struct RHITextureViewInfo 
{
    RHITextureRef texture;
    TextureFormat format = FORMAT_UKNOWN;			// 此时取texture的format
	TextureViewType viewType = VIEW_TYPE_2D;

    TextureSubresourceRange subresource = {};	// 此时取texture的默认range

} RHITextureViewInfo;

typedef struct RHISamplerInfo 
{
	FilterType minFilter = FILTER_TYPE_LINEAR;
    FilterType magFilter = FILTER_TYPE_LINEAR;
    MipMapMode mipmapMode = MIPMAP_MODE_LINEAR;
    AddressMode addressModeU = ADDRESS_MODE_REPEAT;
    AddressMode addressModeV = ADDRESS_MODE_REPEAT;
    AddressMode addressModeW = ADDRESS_MODE_REPEAT;
    CompareFunction compareFunction = COMPARE_FUNCTION_NEVER;

	float mipLodBias = 0.0f;
    float maxAnisotropy = 0.0f;

} RHISamplerInfo;

typedef struct RHIShaderInfo
{
	std::string entry = "main";

	ShaderFrequency frequency;
	std::vector<uint8_t> code;
} RHIShaderInfo;

typedef struct ShaderResourceEntry 
{
	// std::string name;

    uint32_t set = 0;
    uint32_t binding = 0;
    uint32_t size = 1;
	ShaderFrequency frequency = SHADER_FREQUENCY_ALL;

	ResourceType type = RESOURCE_TYPE_NONE;
    // TextureViewType textureViewType = VIEW_TYPE_UNDEFINED;	// 只有反射会填的信息，创建描述符绑定并不需要

	friend bool operator== (const ShaderResourceEntry& a, const ShaderResourceEntry& b)
	{
		return  a.set == b.set &&
				a.binding == b.binding &&
				a.size == b.size &&
				a.frequency == b.frequency &&
				a.type == b.type;
				// a.textureViewType == b.textureViewType;
	}

} ShaderResourceEntry;

typedef struct ShaderReflectInfo
{
    std::string name;

    ShaderFrequency frequency;
    std::vector<ShaderResourceEntry> resources;
    uint32_t localSizeX = 0;
    uint32_t localSizeY = 0;
    uint32_t localSizeZ = 0;

} ShaderReflectInfo;

typedef struct PushConstantInfo
{
	uint32_t size = 128;
	ShaderFrequency frequency;
} PushConstantInfo;

struct AttachmentInfo
{
	RHITextureViewRef	textureView		= nullptr;	

	AttachmentLoadOp 	loadOp          = ATTACHMENT_LOAD_OP_DONT_CARE;
	AttachmentStoreOp	storeOp			= ATTACHMENT_STORE_OP_DONT_CARE;

	Color4				clearColor		= {0.0f, 0.0f, 0.0f, 0.0f};
	float				clearDepth 		= 1.0f;
	uint32_t			clearStencil 	= 0;
};

typedef struct RHIRenderPassInfo
{
	std::array<AttachmentInfo, MAX_RENDER_TARGETS> colorAttachments = {};
	AttachmentInfo depthStencilAttachment = {};

	Extent2D extent = {0, 0};
	uint32_t layers = 1;

} RHIRenderPassInfo;

typedef struct RHIRootSignatureInfo	
{
	// Sakura中使用了shader反射来获取全部的绑定资源信息，在函数CGPUUtil_InitRSParamTables中，
	// 因此初始化创建时只传了着色器而没有绑定的信息
	// 拆分一下把手动创建补上

	RHIRootSignatureInfo& AddPushConstant(const PushConstantInfo& pushConstant) { pushConstants.push_back(pushConstant); return *this; }
	RHIRootSignatureInfo& AddEntry(const ShaderResourceEntry& entry);
	RHIRootSignatureInfo& AddEntry(const RHIRootSignatureInfo& other);
	RHIRootSignatureInfo& AddEntryFromReflect(RHIShaderRef shader);
	const std::vector<PushConstantInfo>& GetPushConstants() const { return pushConstants; };
	const std::vector<ShaderResourceEntry>& GetEntries() const { return entries; };

protected:
	std::vector<ShaderResourceEntry> entries;
	std::vector<PushConstantInfo> pushConstants;
	
} RHIRootSignatureInfo;

typedef struct RHIDescriptorUpdateInfo	
{
	uint32_t binding = 0;
	uint32_t index = 0;

	ResourceType resourceType = RESOURCE_TYPE_NONE;

	RHIBufferRef buffer;
	RHITextureViewRef textureView;
	RHISamplerRef sampler;
	//TODO 光追加速结构

	uint64_t bufferOffset = 0;	// 仅buffer使用
	uint64_t bufferRange = 0;

} RHIDescriptorUpdateInfo;

typedef struct RHIRasterizerStateInfo
{
	RasterizerFillMode fillMode = FILL_MODE_SOLID;
	RasterizerCullMode cullMode = CULL_MODE_BACK;
    RasterizerDepthClipMode depthClipMode = DEPTH_CLIP;

	float depthBias = 0.0f;
	float slopeScaleDepthBias = 0.0f;

	friend bool operator== (const RHIRasterizerStateInfo& a, const RHIRasterizerStateInfo& b)
	{
		return 	a.fillMode 				== b.fillMode &&
				a.cullMode 				== b.cullMode &&
				a.depthClipMode 		== b.depthClipMode &&
				a.depthBias 			== b.depthBias &&
				a.slopeScaleDepthBias 	== b.slopeScaleDepthBias;
	};

} RHIRasterizerStateInfo;

typedef struct RHIDepthStencilStateInfo
{
	bool enableDepthTest = true;
	bool enableDepthWrite = true;
	CompareFunction depthTest = COMPARE_FUNCTION_LESS_EQUAL;

	// bool enableFrontFaceStencil = false;
	// CompareFunction frontFaceStencilTest = COMPARE_FUNCTION_ALWAYS;
	// StencilOp frontFaceStencilFailStencilOp = STENCIL_OP_KEEP;
	// StencilOp frontFaceDepthFailStencilOp = STENCIL_OP_KEEP;
	// StencilOp frontFacePassStencilOp = STENCIL_OP_KEEP;

	// bool enableBackFaceStencil = false;
	// CompareFunction backFaceStencilTest = COMPARE_FUNCTION_ALWAYS;
	// StencilOp backFaceStencilFailStencilOp = STENCIL_OP_KEEP;
	// StencilOp backFaceDepthFailStencilOp = STENCIL_OP_KEEP;
	// StencilOp backFacePassStencilOp = STENCIL_OP_KEEP;

	// uint8_t stencilReadMask = 0xFF;
	// uint8_t stencilWriteMask = 0xFF;
	
	friend bool operator== (const RHIDepthStencilStateInfo& a, const RHIDepthStencilStateInfo& b)
	{	
		return 	a.enableDepthWrite == b.enableDepthWrite &&
				a.depthTest == b.depthTest;
				// a.enableFrontFaceStencil 		== b.enableFrontFaceStencil &&
				// a.frontFaceStencilTest 			== b.frontFaceStencilTest &&
				// a.frontFaceStencilFailStencilOp == b.frontFaceStencilFailStencilOp &&
				// a.frontFaceDepthFailStencilOp 	== b.frontFaceDepthFailStencilOp &&
				// a.frontFacePassStencilOp 		== b.frontFacePassStencilOp &&
				// a.enableBackFaceStencil 		== b.enableBackFaceStencil &&
				// a.backFaceStencilTest 			== b.backFaceStencilTest &&
				// a.backFaceStencilFailStencilOp 	== b.backFaceStencilFailStencilOp &&
				// a.backFaceDepthFailStencilOp 	== b.backFaceDepthFailStencilOp &&
				// a.backFacePassStencilOp 		== b.backFacePassStencilOp &&
				// a.stencilReadMask 				== b.stencilReadMask &&
				// a.stencilWriteMask 				== b.stencilWriteMask;
	}

} RHIDepthStencilStateInfo;

typedef struct RHIBlendStateInfo
{
	struct RenderTarget
	{
		bool enable = false;

		BlendOp colorBlendOp = BLEND_OP_ADD;
		BlendFactor colorSrcBlend = BLEND_FACTOR_ONE;
		BlendFactor colorDstBlend = BLEND_FACTOR_ZERO;

		BlendOp alphaBlendOp = BLEND_OP_ADD;
		BlendFactor alphaSrcBlend = BLEND_FACTOR_ONE;
		BlendFactor alphaDstBlend = BLEND_FACTOR_ZERO;

		ColorWriteMasks colorWriteMask = COLOR_MASK_RGBA;
	};

	std::array<RenderTarget, MAX_RENDER_TARGETS> renderTargets;
	
	friend bool operator== (const RHIBlendStateInfo::RenderTarget& a, const RHIBlendStateInfo::RenderTarget& b)
	{
		return 	a.enable 			== b.enable &&
				a.colorBlendOp 		== b.colorBlendOp &&
				a.colorSrcBlend 	== b.colorSrcBlend &&
				a.colorDstBlend 	== b.colorDstBlend &&				
				a.alphaBlendOp 		== b.alphaBlendOp &&
				a.alphaSrcBlend 	== b.alphaSrcBlend &&
				a.alphaDstBlend 	== b.alphaDstBlend &&
				a.colorWriteMask 	== b.colorWriteMask;
	}

	friend bool operator== (const RHIBlendStateInfo& a, const RHIBlendStateInfo& b)
	{
		// if(a.renderTargets.size() != b.renderTargets.size()) return false;

		for(int32_t i = 0; i < a.renderTargets.size(); i++)
		{
			if (!(a.renderTargets[i] == b.renderTargets[i])) return false;
		}
		return true;
	}

} RHIBlendStateInfo;

typedef struct VertexElement
{
public:

	uint8_t streamIndex = 0;
	uint8_t attributeIndex = 0;
	TextureFormat format = FORMAT_UKNOWN;

	uint8_t offset = 0;
	uint16_t stride = 0;
	bool useInstanceIndex = false;

	friend bool operator== (const VertexElement& a, const VertexElement& b) 
	{
		return  a.streamIndex		    == b.streamIndex &&
				a.attributeIndex	    == b.attributeIndex &&		
				a.format			    == b.format &&
				a.offset			    == b.offset &&
				a.stride			    == b.stride &&
				a.useInstanceIndex    	== b.useInstanceIndex;
	}

} VertexElement;

typedef struct VertexInputStateInfo
{
	std::vector<VertexElement> vertexElements;
} VertexInputStateInfo;

typedef struct RHIGraphicsPipelineInfo
{
	RHIGraphicsPipelineInfo() = default;

	RHIShaderRef					vertexShader;
	RHIShaderRef					geometryShader;
	RHIShaderRef	 				fragmentShader;

	RHIRootSignatureRef				rootSignature;

	VertexInputStateInfo            vertexInputState = {};
	PrimitiveType					primitiveType = PRIMITIVE_TYPE_TRIANGLE_LIST;
	RHIRasterizerStateInfo			rasterizerState = {};
	RHIBlendStateInfo				blendState = {};
	RHIDepthStencilStateInfo		depthStencilState = {};

	std::array<TextureFormat, MAX_RENDER_TARGETS> colorAttachmentFormats = { FORMAT_UKNOWN };
	TextureFormat						depthStencilAttachmentFormat = FORMAT_UKNOWN;

	// uint32_t						numSamples = 1;		// TODO
	// uint8_t						subpassIndex = 0;	// 放弃支持sub pass

} RHIGraphicsPipelineInfo;

typedef struct RHIComputePipelineInfo
{
	RHIShaderRef 					computeShader;

	RHIRootSignatureRef				rootSignature;

} RHIComputePipelineInfo;

typedef struct RHIRayTracingPipelineInfo
{
	RHIShaderRef 					rayGenShader;
	RHIShaderRef					anyHitShader;
	RHIShaderRef					closestHitShader;
	RHIShaderRef					rayMissShader;
	RHIShaderRef					intersectionShader;

	RHIRootSignatureRef				rootSignature;

} RHIRayTracingPipelineInfo;

typedef struct RHIBufferBarrier 
{
    RHIBufferRef buffer;
    RHIResourceState srcState;
    RHIResourceState dstState;

	uint32_t offset = 0;
	uint32_t size = 0;

} RHIBufferBarrier;

typedef struct RHITextureBarrier
{
	RHITextureRef texture;
    RHIResourceState srcState;
    RHIResourceState dstState;

	TextureSubresourceRange subresource = {};	// 此时取texture的默认range

} RHITextureBarrier;
