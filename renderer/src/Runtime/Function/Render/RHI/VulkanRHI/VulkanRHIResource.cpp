#include "VulkanRHIResource.h"
#include "VulkanUtil.h"
#include "VulkanRHI.h"
#include "Function/Render/RHI/RHIResource.h"
#include "Function/Render/RHI/RHIStructs.h"
#include "Core/Log/Log.h"

#include <regex>
#include <spirv_reflect.h>
#include <memory>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

//基本资源 ////////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanRHISurface::VulkanRHISurface(GLFWwindow* window, VulkanRHIBackend& backend)
: RHISurface()
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    extent = { (uint32_t)width, (uint32_t)height };

    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    //window = glfwCreateWindow(extent.width, extent.height, "", nullptr, nullptr);

    //glfwSetWindowUserPointer(window, this);
    //glfwSetWindowSizeCallback(window, nullptr); //TODO
    //glfwSetCursorPosCallback(window, nullptr);

    if (glfwCreateWindowSurface(backend.GetInstance(), window, nullptr, &handle) != VK_SUCCESS) 
    {
        LOG_FATAL("Failed to create window surface!");
    }
}

void VulkanRHISurface::Destroy() 
{ 
    vkDestroySurfaceKHR(Backend()->GetInstance(), handle, nullptr); 
} 

VulkanRHISwapchain::VulkanRHISwapchain(const RHISwapchainInfo& info, VulkanRHIBackend& backend)
: RHISwapchain(info)
{
    VkPhysicalDevice device = backend.GetPhysicalDevice();
    VkDevice logicalDevice = backend.GetLogicalDevice();
    VkSurfaceKHR surface = ResourceCast(info.surface)->GetHandle();

    // 设备支持信息
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

    uint32_t size;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &size, nullptr);
    availableFormats.resize(size);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &size, availableFormats.data());

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &size, nullptr);
    availablePresentModes.resize(size);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &size, availablePresentModes.data());

    // 交换链基本信息
    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(VulkanUtil::RHIFormatToVkFormat(info.format));
    RHIFormat targetFormat = VulkanUtil::VkFormatToRHIFormat(surfaceFormat.format);
    if(targetFormat != info.format)
    {
        this->info.format = targetFormat;
        LOG_FATAL("Cant find swapchain image format support!");
    }

    VkPresentModeKHR presentMode = ChooseSwapPresentMode();

    VkExtent2D extent = ChooseSwapExtent();
    if(extent.width != info.extent.width || extent.height != info.extent.height) 
    {
        this->info.extent = { extent.width, extent.height };
        LOG_FATAL("Cant find suitable swapchain image extent!");
    }

    // 交换链图像数目
    uint32_t imageCount = std::max(info.imageCount, capabilities.minImageCount);
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) 
    {
        imageCount = capabilities.maxImageCount;
    }
    if(info.imageCount != imageCount) 
    {
        this->info.imageCount = capabilities.maxImageCount;
        LOG_DEBUG("Swapchain image count is greater than capability maximum!");
    }

    // 创建交换链信息
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | 
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
                            VK_IMAGE_USAGE_SAMPLED_BIT | 
                            VK_IMAGE_USAGE_STORAGE_BIT;

    // 检测队列族对交换链图像的操作方式
    /*
    uint32_t queueFamilyIndices[] = { (uint32_t)queueInfo.graphicsFamily, (uint32_t)queueInfo.presentFamily };
    if (queueInfo.graphicsFamily != queueInfo.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;   // 图像可被多个队列族访问
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;    // 图像同一时间只能被单个队列族访问
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }
    */
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;    // 图像同一时间只能被单个队列族访问
    createInfo.queueFamilyIndexCount = 0; // Optional
    createInfo.pQueueFamilyIndices = nullptr; // Optional

    createInfo.preTransform = capabilities.currentTransform;                    // 变换操作，例如旋转反转，用默认
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;              // 透明度混合
    createInfo.presentMode = presentMode;                                       // 刷新模式
    createInfo.clipped = VK_TRUE;                                               // 裁剪（遮挡或再可视范围外等）
    createInfo.oldSwapchain = VK_NULL_HANDLE;                                   // 交换链更新时使用

    if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &handle) != VK_SUCCESS)
    {
        LOG_FATAL("Failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(logicalDevice, handle, &imageCount, nullptr);   // 获取image句柄
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(logicalDevice, handle, &imageCount, images.data());

    imageFormat = surfaceFormat.format;  // 存储extent和format
    imageExtent = extent;

    for(uint32_t i = 0; i < imageCount; i++)
    {
        RHITextureInfo info = {
            .format = targetFormat,
            .extent = { extent.width, extent.height, 1},
            .arrayLayers = 1,
            .mipLevels = 1,
            .memoryUsage = MEMORY_USAGE_GPU_ONLY,
            .type = RESOURCE_TYPE_TEXTURE | RESOURCE_TYPE_RENDER_TARGET,
            .creationFlag = TEXTURE_CREATION_NONE
        };

        RHITextureRef texture = std::make_shared<VulkanRHITexture>(info, backend, images[i]);
        textures.push_back(texture);

        // 留着RESOURCE_STATE_UNDEFINED之后处理其实也可以，可加可不加
        backend.GetImmediateCommand()->TextureBarrier(
            {texture, RESOURCE_STATE_UNDEFINED, RESOURCE_STATE_PRESENT, {TEXTURE_ASPECT_COLOR, 0, 1, 0, 1}});
        backend.GetImmediateCommand()->Flush();
    }
}

RHITextureRef VulkanRHISwapchain::GetNewFrame(RHIFenceRef fence, RHISemaphoreRef signalSemaphore) 
{
    VkFence signalFence = VK_NULL_HANDLE;
    VkSemaphore semaphore = VK_NULL_HANDLE;

    if(fence != nullptr) signalFence = ResourceCast(fence)->GetHandle();
    if(signalSemaphore != nullptr) semaphore = ResourceCast(signalSemaphore)->GetHandle();

	VkResult result = vkAcquireNextImageKHR(Backend()->GetLogicalDevice(),      //TODO 窗口变化
    handle, UINT64_MAX, semaphore, signalFence, &currentIndex);

    return textures[currentIndex];
}

void VulkanRHISwapchain::Present(RHISemaphoreRef waitSemaphore)
{
    VkSemaphore semaphore = VK_NULL_HANDLE;
    if(waitSemaphore != nullptr) semaphore = ResourceCast(waitSemaphore)->GetHandle();

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &handle;
    presentInfo.pImageIndices = &currentIndex;
    presentInfo.pResults = nullptr;
    presentInfo.waitSemaphoreCount = semaphore == VK_NULL_HANDLE ? 0 : 1;
    presentInfo.pWaitSemaphores = &semaphore;

    if (vkQueuePresentKHR(ResourceCast(info.presentQueue)->GetHandle(), &presentInfo) != VK_SUCCESS) 
    {
        LOG_FATAL("Failed to present swap chain image!\n");
    }
}

VkSurfaceFormatKHR VulkanRHISwapchain::ChooseSwapSurfaceFormat(VkFormat targetFormat) 
{
    // 选择通道标准，以及色彩空间
    std::cout << "Available swapchain surface formats:" << std::endl;
    for (const auto format : availableFormats) {
        std::cout << format.format << " : " << format.colorSpace << std::endl;
    }
    std::cout << " " << std::endl;

    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {    //无偏向性，任选
        return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    }

    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == targetFormat && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR VulkanRHISwapchain::ChooseSwapPresentMode() 
{
    // 选择刷新模式
    std::cout << "Available swapchain present modes:" << std::endl;
    for (const auto mode : availablePresentModes) {
        std::cout << mode << std::endl;
    }
    std::cout << " " << std::endl;

    VkPresentModeKHR bestMode;
    bestMode = VK_PRESENT_MODE_IMMEDIATE_KHR;         // normal
    //bestMode = VK_PRESENT_MODE_MAILBOX_KHR;             // low latency
    //bestMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;      // minimize stuttering
    //bestMode = VK_PRESENT_MODE_FIFO_KHR;              // low power consumption
    
    // for (const auto& availablePresentMode : presentModes) {
    //     if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
    //         return availablePresentMode;
    //     }
    //     else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
    //         bestMode = availablePresentMode;
    //     }
    // }

    return bestMode;
}

VkExtent2D VulkanRHISwapchain::ChooseSwapExtent() 
{
    // 选择分辨率
    std::cout << "Swapchain extent: " << capabilities.currentExtent.width << " : " << capabilities.currentExtent.height << std::endl;

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
    {
        return capabilities.currentExtent;
    }
    else {
        //int width, height;
        //glfwGetWindowSize(pWindow, &width, &height);
        VkExtent2D actualExtent = { info.extent.width, info.extent.height };

        //std::cout << width << " " << height << std::endl;

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

void VulkanRHISwapchain::Destroy() 
{ 
    vkDestroySwapchainKHR(Backend()->GetLogicalDevice(), handle, nullptr); 
} 

VulkanRHICommandPool::VulkanRHICommandPool(const RHICommandPoolInfo& info, VulkanRHIBackend& backend)
: RHICommandPool(info)
{
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = ResourceCast(info.queue)->GetQueueFamilyIndex();  //命令池需要绑定队列族，使用其指定的命令类型
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

    if (vkCreateCommandPool(backend.GetLogicalDevice(), &poolInfo, nullptr, &handle) != VK_SUCCESS) 
    {
        LOG_FATAL("Failed to create command pool!");
    }
}

void VulkanRHICommandPool::Destroy()
{
    vkDestroyCommandPool(Backend()->GetLogicalDevice(), handle, nullptr);
}

//缓冲，纹理 ////////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanRHIBuffer::VulkanRHIBuffer(const RHIBufferInfo& info, VulkanRHIBackend& backend)
: RHIBuffer(info)
{
    VkBufferUsageFlags usage = VulkanUtil::ResourceTypeToBufferUsage(info.type);
    if (info.memoryUsage == MEMORY_USAGE_GPU_ONLY || info.memoryUsage == MEMORY_USAGE_GPU_TO_CPU)   usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = info.size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0,
    bufferInfo.pQueueFamilyIndices = NULL;

    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VulkanUtil::MemoryUsageToVma(info.memoryUsage);
    if(info.creationFlag & BUFFER_CREATION_PERSISTENT_MAP) 
    {
        allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        mapped = true;
    }
    //allocationCreateInfo.requiredFlags    //必要需求
    //allocationCreateInfo.preferredFlags   //尽量满足的需求

    //VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT 强制要求单独开辟内存
    //VMA_ALLOCATION_CREATE_MAPPED_BIT  强制持久映射，可由allocationInfo.pMappedData直接访问
    //vmaFlushAllocation(), vmaInvalidateAllocation() 指定缓存写入和缓存失效

    allocationInfo = {};
    if(vmaCreateBuffer(backend.GetMemoryAllocator(), &bufferInfo, &allocationCreateInfo, &handle, &allocation, &allocationInfo) != VK_SUCCESS)
    {
        LOG_FATAL("VMA failed to allocate buffer!");
    }

    //vmaMapMemory(VmaAllocator  _Nonnull allocator, VmaAllocation  _Nonnull allocation, void * _Nullable * _Nonnull ppData)
}

void* VulkanRHIBuffer::Map()
{
    if(info.creationFlag & BUFFER_CREATION_PERSISTENT_MAP) return allocationInfo.pMappedData;
    if(!mapped) 
    {
        vmaMapMemory(Backend()->GetMemoryAllocator(), allocation, &pointer);
        mapped = true;
    }
    return pointer;
}

void VulkanRHIBuffer::UnMap()
{
    if(mapped && !(info.creationFlag & BUFFER_CREATION_PERSISTENT_MAP))
    {
        vmaUnmapMemory(Backend()->GetMemoryAllocator(), allocation);
        pointer = nullptr;
        mapped = false;
    }
}

// void VulkanRHIBuffer::FlushRange(uint32_t size, uint32_t offset)
// {
//     uint32_t realSize = (size == 0) ? info.size - offset : size;
//     //vkFlushMappedMemoryRanges()
//     vmaFlushAllocation(Backend()->GetMemoryAllocator(), allocation, offset, realSize);
// }

void VulkanRHIBuffer::Destroy()
{
    vmaDestroyBuffer(Backend()->GetMemoryAllocator(), handle, allocation);
}

VulkanRHITexture::VulkanRHITexture(const RHITextureInfo& info, VulkanRHIBackend& backend, VkImage image)
: RHITexture(info)
{
    TextureAspectFlags aspects =    IsDepthStencilFormat(info.format) ? TEXTURE_ASPECT_DEPTH_STENCIL :
                                    IsDepthFormat(info.format) ? TEXTURE_ASPECT_DEPTH :
                                    IsStencilFormat(info.format) ? TEXTURE_ASPECT_STENCIL : TEXTURE_ASPECT_COLOR;
    defaultRange = {aspects, 0, info.mipLevels, 0, info.arrayLayers};
    defaultLayers = {aspects, 0, 0, info.arrayLayers};

    if(image != VK_NULL_HANDLE)     // 留给swapchain的初始化方式
    {
        handle = image; 
        return;
    }

    VkFormat format = VulkanUtil::RHIFormatToVkFormat(info.format);

    VkImageUsageFlags usage = VulkanUtil::ResourceTypeToImageUsage(info.type);
    if(IsDepthFormat(info.format) || IsStencilFormat(info.format))   usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    else if(info.type & RESOURCE_TYPE_RENDER_TARGET)                                usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    

    VkImageType type =  info.extent.depth > 1 ? VK_IMAGE_TYPE_3D :
                        info.extent.height > 1 ? VK_IMAGE_TYPE_2D : 
                        VK_IMAGE_TYPE_1D;
    if(info.creationFlag & TEXTURE_CREATION_FORCE_2D) type = VK_IMAGE_TYPE_2D;
    if(info.creationFlag & TEXTURE_CREATION_FORCE_3D) type = VK_IMAGE_TYPE_3D;

    VkImageCreateFlags flag = 0;
    if(info.type & RESOURCE_TYPE_TEXTURE_CUBE)      flag |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    if(type == VK_IMAGE_TYPE_3D)                    flag |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR;


    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = type;
    imageInfo.extent.width = info.extent.width;
    imageInfo.extent.height = info.extent.height;
    imageInfo.extent.depth = info.extent.depth;
    imageInfo.mipLevels = info.mipLevels;
    imageInfo.arrayLayers = info.arrayLayers;   
    if(info.type & RESOURCE_TYPE_TEXTURE_CUBE) imageInfo.arrayLayers = std::max(imageInfo.arrayLayers, (uint32_t)6);
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = usage;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = flag; // Optional

    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VulkanUtil::MemoryUsageToVma(info.memoryUsage);

    allocationInfo = {};
    if(vmaCreateImage(backend.GetMemoryAllocator(), &imageInfo, &allocationCreateInfo, &handle, &allocation, &allocationInfo) != VK_SUCCESS)
    {
        LOG_FATAL("VMA failed to allocate image!");
    }
}

void VulkanRHITexture::Destroy()
{
    vmaDestroyImage(Backend()->GetMemoryAllocator(), handle, allocation);
    //vkDestroyImage(Backend()->GetLogicalDevice(), handle, nullptr);
}

VulkanRHITextureView::VulkanRHITextureView(const RHITextureViewInfo& info, VulkanRHIBackend& backend)
: RHITextureView(info)
{
    if(info.subresource.aspect == TEXTURE_ASPECT_NONE)  this->info.subresource = info.texture->GetDefaultSubresourceRange();

    RHITextureInfo textureInfo = ResourceCast(info.texture)->GetInfo();
    VkImageAspectFlags aspectMask = VulkanUtil::TextureAspectToVk(this->info.subresource.aspect);

    // if(IsDepthStencilFormat(info.format)) 
    // {
    //     aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
    //     if(IsStencilFormat(info.format))        aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    // }
    // else 
    // {
    //     if(textureInfo.type & RESOURCE_TYPE_TEXTURE)    aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
    //     if(textureInfo.type & RESOURCE_TYPE_RW_TEXTURE) aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
    // }

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = ResourceCast(info.texture)->GetHandle();
    viewInfo.viewType = VulkanUtil::TextureViewTypeToVk(info.viewType);  
    viewInfo.format = VulkanUtil::RHIFormatToVkFormat(info.format);                  
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;    
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.subresourceRange.aspectMask = aspectMask;
    viewInfo.subresourceRange.baseArrayLayer = this->info.subresource.baseArrayLayer;
    viewInfo.subresourceRange.baseMipLevel = this->info.subresource.baseMipLevel;
    viewInfo.subresourceRange.layerCount = this->info.subresource.layerCount;
    viewInfo.subresourceRange.levelCount = this->info.subresource.levelCount;

    if (vkCreateImageView(backend.GetLogicalDevice(), &viewInfo, nullptr, &handle) != VK_SUCCESS) 
    {
        LOG_FATAL("Failed to create texture image view!");
    }
}

void VulkanRHITextureView::Destroy()
{
    vkDestroyImageView(Backend()->GetLogicalDevice(), handle, nullptr);
}

VulkanRHISampler::VulkanRHISampler(const RHISamplerInfo& info, VulkanRHIBackend& backend)
: RHISampler(info)
{
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VulkanUtil::FilterTypeToVk(info.magFilter);   //放大滤波  //LOD较小（较近）时使用？？
    samplerInfo.minFilter = VulkanUtil::FilterTypeToVk(info.minFilter);   //缩小滤波
    samplerInfo.mipmapMode = VulkanUtil::MipMapModeToVk(info.mipmapMode);
    samplerInfo.anisotropyEnable = info.maxAnisotropy > 0.0f ? VK_TRUE : VK_FALSE;
    samplerInfo.maxAnisotropy = info.maxAnisotropy;
    samplerInfo.minLod = 0.0f;                           
    samplerInfo.maxLod = 100.0f;
    samplerInfo.mipLodBias = info.mipLodBias;              
    samplerInfo.addressModeU = VulkanUtil::AddressModeToVk(info.addressModeU);
    samplerInfo.addressModeV = VulkanUtil::AddressModeToVk(info.addressModeV);
    samplerInfo.addressModeW = VulkanUtil::AddressModeToVk(info.addressModeW);
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    samplerInfo.compareEnable = info.compareFunction == COMPARE_FUNCTION_NEVER ? VK_FALSE : VK_TRUE;
    samplerInfo.compareOp = VulkanUtil::CompareFunctionToVk(info.compareFunction);

    // VkPhysicalDeviceProperties properties{};
    // vkGetPhysicalDeviceProperties(Backend::Get()->physicalDevice, &properties);
    // samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; //设备支持的最大各向异性滤波采样数目
    // if (maxAnisotropy > 0)
    // {
    //     samplerInfo.anisotropyEnable = VK_TRUE;             //各向异性滤波
    //     samplerInfo.maxAnisotropy = (float)maxAnisotropy;   //设备支持的最大各向异性滤波采样数目
    // }
    // else
    // {
    //     samplerInfo.anisotropyEnable = VK_FALSE;
    // }

    //add a extension struct to enable Min mode
    VkSamplerReductionModeCreateInfoEXT createInfoReduction = {};
    createInfoReduction.sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO_EXT;
    createInfoReduction.reductionMode = VulkanUtil::SamplerReductionModeToVk(info.reductionMode);
    samplerInfo.pNext = &createInfoReduction;

    if (vkCreateSampler(backend.GetLogicalDevice(), &samplerInfo, nullptr, &handle) != VK_SUCCESS) 
    {
        LOG_FATAL("Failed to create texture sampler!");
    }
}

void VulkanRHISampler::Destroy()
{
    vkDestroySampler(Backend()->GetLogicalDevice(), handle, nullptr);
}

//着色器 ////////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanRHIShader::VulkanRHIShader(const RHIShaderInfo& info, VulkanRHIBackend& backend)
: RHIShader(info)
{
    // 从spv文件的字符串信息里收集定义的宏
    std::regex pattern("#define (\\w+)");
    for (std::cregex_iterator it((char*)info.code.data(), (char*)info.code.data() + info.code.size(), pattern); 
            it != std::cregex_iterator{}; it++) 
    {
        reflectInfo.definedSymbols.insert((*it)[1].str());
    }

    // 创建ShaderModule
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = info.code.size();
    createInfo.pCode = (const uint32_t*)info.code.data();

    if (vkCreateShaderModule(backend.GetLogicalDevice(), &createInfo, nullptr, &handle) != VK_SUCCESS) 
    {
        LOG_FATAL("Failed to create shader module!");
    }
    this->info.code.clear();    // 代码不需要带着了

    // 收集反射信息
    SpvReflectShaderModule module;
    SpvReflectResult result = spvReflectCreateShaderModule(info.code.size(), info.code.data(), &module);
    if(result != SPV_REFLECT_RESULT_SUCCESS)    LOG_DEBUG("Failed to generate shader reflect data!");
    if(module.entry_point_count != 1)           LOG_DEBUG("Shader file contains more than one entry!");   //暂时只做单entry吧       

    const SpvReflectEntryPoint* entry = spvReflectGetEntryPoint(&module, module.entry_points[0].name);

    reflectInfo.name = std::string(entry->name);
    reflectInfo.frequency = VulkanUtil::SpvShaderStageToFrequency(entry->shader_stage);
    if (reflectInfo.frequency == SHADER_FREQUENCY_COMPUTE)
    {
        reflectInfo.localSizeX = entry->local_size.x;
        reflectInfo.localSizeY = entry->local_size.y;
        reflectInfo.localSizeZ = entry->local_size.z;
    }

    // bool isGLSL = module.source_language & SpvSourceLanguageGLSL;
    // bool isHLSL = module.source_language & SpvSourceLanguageHLSL;

    // pushConstant
    // uint32_t pushConstantCnt;
    // spvReflectEnumeratePushConstantBlocks(&module, &pushConstantCnt, NULL);
    // if (pushConstantCnt > 0) 
    // {
    //     std::vector<SpvReflectBlockVariable*> blockVariables(pushConstantCnt + 1);
    //     spvReflectEnumeratePushConstantBlocks(&module, &pushConstantCnt, blockVariables.data());
    // }

    // 着色器输入和输出
    uint32_t inputVariableCnt;
    spvReflectEnumerateInputVariables(&module, &inputVariableCnt, NULL);
    if (inputVariableCnt > 0)
    {
        std::vector<SpvReflectInterfaceVariable*> inputVariables(inputVariableCnt);   
        spvReflectEnumerateInputVariables(&module, &inputVariableCnt, inputVariables.data()); 

        for(uint32_t i = 0; i < inputVariableCnt; i++)
        {
            if(inputVariables[i]->location < MAX_SHADER_IN_OUT_VARIABLES)
                reflectInfo.inputVariables[inputVariables[i]->location] = VulkanUtil::SpvFormatToRHIFormat(inputVariables[i]->format);
        }
    }

    uint32_t outputVariableCnt;
    spvReflectEnumerateOutputVariables(&module, &outputVariableCnt, NULL);
    if(outputVariableCnt > 0)
    {
        std::vector<SpvReflectInterfaceVariable*> outputVariables(outputVariableCnt);
        spvReflectEnumerateOutputVariables(&module, &outputVariableCnt, outputVariables.data());

        for(uint32_t i = 0; i < outputVariableCnt; i++)
        {
            if(outputVariables[i]->location < MAX_SHADER_IN_OUT_VARIABLES)
                reflectInfo.outputVariables[outputVariables[i]->location] = VulkanUtil::SpvFormatToRHIFormat(outputVariables[i]->format);
        }
    }

    // specialization constant
    // uint32_t specializationConstantCnt;
    // spvReflectEnumerateSpecializationConstants(&module, &specializationConstantCnt, NULL);
    // if(specializationConstantCnt > 0)
    // {
    //     std::vector<SpvReflectSpecializationConstant*> specializationConstants(specializationConstantCnt);
    //     spvReflectEnumerateSpecializationConstants(&module, &specializationConstantCnt, specializationConstants.data());

    //     for(uint32_t i = 0; i < specializationConstantCnt; i++)
    //     {
    //         specializationConstants[i];
    //     }       
    // }

    // interface variable
    // uint32_t interfaceVariableCnt;
    // spvReflectEnumerateInterfaceVariables(&module, &interfaceVariableCnt, NULL);
    // if(interfaceVariableCnt > 0)
    // {
    //     std::vector<SpvReflectInterfaceVariable*> interfaceVariables(interfaceVariableCnt);
    //     spvReflectEnumerateInterfaceVariables(&module, &interfaceVariableCnt, interfaceVariables.data());

    //     for(uint32_t i = 0; i < interfaceVariableCnt; i++)
    //     {
    //         interfaceVariables[i];
    //     }       
    // }


    // 描述符
    uint32_t descriptorSetCnt;
    spvReflectEnumerateDescriptorSets(&module, &descriptorSetCnt, NULL);
    if (descriptorSetCnt > 0)
    {
        std::vector<SpvReflectDescriptorSet*> descriptorSets(descriptorSetCnt);
        spvReflectEnumerateDescriptorSets(&module, &descriptorSetCnt, descriptorSets.data());
        
        uint32_t descriptorSize = 0;
        for (uint32_t i = 0; i < descriptorSetCnt; i++)   descriptorSize += descriptorSets[i]->binding_count;
        reflectInfo.resources.resize(descriptorSize);

        uint32_t i = 0;
        for (uint32_t set = 0; set < descriptorSetCnt; set++)
        {
            SpvReflectDescriptorSet* currentSet = descriptorSets[set];

            for (uint32_t binding = 0; binding < currentSet->binding_count; binding++, i++)
            {
                SpvReflectDescriptorBinding* currentBinding = currentSet->bindings[binding];

                ShaderResourceEntry& entry = reflectInfo.resources[i];
                //entry.name = std::string(currentBinding->name);
                entry.set = currentBinding->set;
                entry.binding = currentBinding->binding; 
                entry.size = currentBinding->count;
                entry.type = VulkanUtil::SpvDescriptorTypeToResourceType(currentBinding->descriptor_type);
                entry.frequency = reflectInfo.frequency;

                if ((currentBinding->type_description->type_flags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_IMAGE) ||
                    (currentBinding->type_description->type_flags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLED_IMAGE))
                {
                    bool isArray = (currentBinding->type_description->type_flags & SPV_REFLECT_TYPE_FLAG_ARRAY);
                    // entry.textureViewType = VulkanUtil::SpvDimToTextureViewType(currentBinding->image.dim, isArray); // 暂时不需要这个信息
                }
            }
        }
    }
}

VkPipelineShaderStageCreateInfo VulkanRHIShader::GetShaderStageCreateInfo()
{
    VkPipelineShaderStageCreateInfo shaderStage = {};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = VulkanUtil::ShaderFrequencyToVkStageFlagBits(info.frequency);
    shaderStage.module = handle;
    shaderStage.pName = info.entry.c_str();

    return shaderStage;
}

void VulkanRHIShader::Destroy()
{
    vkDestroyShaderModule(Backend()->GetLogicalDevice(), handle, nullptr);
}

VulkanRHIRootSignature::VulkanRHIRootSignature(const RHIRootSignatureInfo& info, VulkanRHIBackend& backend)
: RHIRootSignature(info)
{
    for(const ShaderResourceEntry& entry : info.GetEntries())
    {
        //描述符布局绑定信息
        VkDescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding = entry.binding;
        layoutBinding.stageFlags = VulkanUtil::ShaderFrequencyToVkStageFlags(entry.frequency);
        layoutBinding.descriptorType = VulkanUtil::ResourceTypeToVk(entry.type);
        layoutBinding.descriptorCount = entry.size == 0 ? 8192 : entry.size;    //指定该绑定处的描述符数量，>1为数组（一个layout多个binding，一个binding多个descriptor）
                                                                                //开启扩展后为最大可能的数量，且这种binding必须在layout的最后  
        layoutBinding.pImmutableSamplers = nullptr;

        if(setInfos.size() < entry.set + 1) setInfos.resize(entry.set + 1);
        setInfos[entry.set].bindings.push_back(layoutBinding);
    }

    for(SetInfo& set : setInfos)
    {
        if (set.bindings.size() > 0) 
        {
            //描述符布局信息
            VkDescriptorSetLayoutCreateInfo layoutInfo;
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = (uint32_t)set.bindings.size();
            layoutInfo.pBindings = set.bindings.data();
            layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;  //TODO 使得描述符可以实时更新

            // 启用可变大小描述符数量标志位
            //VkDescriptorBindingFlagsEXT descriptorBindingFlags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;
            std::vector<VkDescriptorBindingFlagsEXT> descriptorBindingFlags = {};
            descriptorBindingFlags.resize((uint32_t)set.bindings.size());
            for (auto& descriptorBindingFlag : descriptorBindingFlags)
            {
                descriptorBindingFlag = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;  //允许 Variable Descriptor binding 的 Descriptor 在没有被动态访问时不指定为有效的描述符
            }
            
            // 用于bindless创建可变的binding descriptor数目
            VkDescriptorSetLayoutBindingFlagsCreateInfo setLayoutBindingFlags{};
            setLayoutBindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
            setLayoutBindingFlags.bindingCount = (uint32_t)set.bindings.size();
            setLayoutBindingFlags.pBindingFlags = descriptorBindingFlags.data();

            // 指定 Descriptor Set Layout CreateInfo 扩展
            layoutInfo.pNext = &setLayoutBindingFlags;

            if (vkCreateDescriptorSetLayout(backend.GetLogicalDevice(), &layoutInfo, nullptr, &set.layout) != VK_SUCCESS) 
            {
                LOG_FATAL("Failed to create descriptor set layout!");
            }
        }
    }
}

RHIDescriptorSetRef VulkanRHIRootSignature::CreateDescriptorSet(uint32_t set)
{
    if(setInfos.size() > set && setInfos[set].bindings.size() > 0)
    {
        RHIDescriptorSetRef descriptorSet = std::make_shared<VulkanRHIDescriptorSet>(setInfos[set].layout, *Backend());  
        Backend()->RegisterResource(descriptorSet);

        return descriptorSet;
    }

    LOG_DEBUG("Unable to find descriptor info!");
    return nullptr;
}

void VulkanRHIRootSignature::Destroy()
{
    for(SetInfo& set : setInfos)
    {
        vkDestroyDescriptorSetLayout(Backend()->GetLogicalDevice(), set.layout, nullptr);
    } 
}

VulkanRHIDescriptorSet::VulkanRHIDescriptorSet(VkDescriptorSetLayout setLayout, VulkanRHIBackend& backend)
: RHIDescriptorSet()
{
    //描述符集合信息
    VkDescriptorSetLayout layouts[] = { setLayout };

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = backend.GetDescriptorPool();      //指定描述符池
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts;                                //指定描述符集合的布局

    if (vkAllocateDescriptorSets(backend.GetLogicalDevice(), &allocInfo, &handle) != VK_SUCCESS) 
    {
        LOG_FATAL("Failed to allocate descriptor set!");
    }
}

RHIDescriptorSet& VulkanRHIDescriptorSet::UpdateDescriptor(const RHIDescriptorUpdateInfo& descriptorUpdateInfo)
{
    //更新写入信息
    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = handle;
    descriptorWrite.dstBinding = descriptorUpdateInfo.binding;
    descriptorWrite.dstArrayElement = descriptorUpdateInfo.index;
    descriptorWrite.descriptorType = VulkanUtil::ResourceTypeToVk(descriptorUpdateInfo.resourceType);
    descriptorWrite.descriptorCount = 1;
    
    VkDescriptorImageInfo imageDescriptor = {};
    VkDescriptorBufferInfo bufferDescriptor = {};
    VkWriteDescriptorSetAccelerationStructureKHR accelerationDescriptor = {};

    switch (descriptorUpdateInfo.resourceType) {

    case RESOURCE_TYPE_SAMPLER:
        imageDescriptor.sampler = ResourceCast(descriptorUpdateInfo.sampler)->GetHandle();
        descriptorWrite.pImageInfo = &imageDescriptor;
        break;
        
    case RESOURCE_TYPE_TEXTURE:
	case RESOURCE_TYPE_RW_TEXTURE:
    case RESOURCE_TYPE_TEXTURE_CUBE:
        imageDescriptor.imageView = ResourceCast(descriptorUpdateInfo.textureView)->GetHandle();
        imageDescriptor.imageLayout = VulkanUtil::ResourceTypeToImageLayout(descriptorUpdateInfo.resourceType);
        descriptorWrite.pImageInfo = &imageDescriptor;
        break;

	case RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER:
        imageDescriptor.sampler = ResourceCast(descriptorUpdateInfo.sampler)->GetHandle();
        imageDescriptor.imageView = ResourceCast(descriptorUpdateInfo.textureView)->GetHandle();
        imageDescriptor.imageLayout = VulkanUtil::ResourceTypeToImageLayout(descriptorUpdateInfo.resourceType);
        descriptorWrite.pImageInfo = &imageDescriptor;
        break;

    case RESOURCE_TYPE_BUFFER:
    case RESOURCE_TYPE_RW_BUFFER:
    case RESOURCE_TYPE_UNIFORM_BUFFER:
        bufferDescriptor.buffer = ResourceCast(descriptorUpdateInfo.buffer)->GetHandle();
        bufferDescriptor.offset = descriptorUpdateInfo.bufferOffset;
        bufferDescriptor.range = (descriptorUpdateInfo.bufferRange > 0) ? descriptorUpdateInfo.bufferRange : VK_WHOLE_SIZE;
        descriptorWrite.pBufferInfo = &bufferDescriptor;
        break;

	// case RESOURCE_TYPE_RAY_TRACING:  // TODO

    default:    LOG_FATAL("Unsupported resource type!");
    }

    vkUpdateDescriptorSets(Backend()->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);

    return *this;
}

void VulkanRHIDescriptorSet::Destroy()
{
    //vkFreeDescriptorSets(Backend()->GetLogicalDevice(), Backend()->GetDescriptorPool(), 1, &handle);
}

//管线状态 ////////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanRHIRenderPass::VulkanRHIRenderPass(const RHIRenderPassInfo& info, VulkanRHIBackend& backend)
: RHIRenderPass(info)
{
    // 无论是vkpipeline需要预创建兼容的pass，还是RDG每帧创建renderpass，都导致了需要在RHI层做池化？

    // 创建renderpass
    std::vector<VkImageView> imageViews;
    VulkanRenderPassAttachments renderPassAttachments = {};
    for(uint32_t i = 0; i < info.colorAttachments.size(); i++)
    {
        if(info.colorAttachments[i].textureView == nullptr) break;  // attachment不允许中间有间隔的空元素，检查到有空就停止
        
        renderPassAttachments.colorAttachments.push_back({
            .format = VulkanUtil::RHIFormatToVkFormat(info.colorAttachments[i].textureView->GetInfo().format),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VulkanUtil::AttachmentLoadOpToVk(info.colorAttachments[i].loadOp),
            .storeOp = VulkanUtil::AttachmentStoreOpToVk(info.colorAttachments[i].storeOp),
        });
        imageViews.push_back(ResourceCast(info.colorAttachments[i].textureView)->GetHandle());
    }
    if(info.depthStencilAttachment.textureView != nullptr)
    {
        renderPassAttachments.depthStencilAttachment = {
            .format = VulkanUtil::RHIFormatToVkFormat(info.depthStencilAttachment.textureView->GetInfo().format),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VulkanUtil::AttachmentLoadOpToVk(info.depthStencilAttachment.loadOp),
            .storeOp = VulkanUtil::AttachmentStoreOpToVk(info.depthStencilAttachment.storeOp),
        };
        imageViews.push_back(ResourceCast(info.depthStencilAttachment.textureView)->GetHandle());
    }
    handle = Backend()->FindOrCreateVkRenderPass(renderPassAttachments);

    // 创建framebuffer
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = handle;
    framebufferInfo.attachmentCount = (uint32_t)imageViews.size();
    framebufferInfo.pAttachments = imageViews.data();
    framebufferInfo.width = info.extent.width;
    framebufferInfo.height = info.extent.height;
    framebufferInfo.layers = info.layers;

    frameBuffer = Backend()->FindOrCreateVkFramebuffer(framebufferInfo);
}

void VulkanRHIRenderPass::Destroy()
{
    // 池化统一删除
    // vkDestroyFramebuffer(Backend()->GetLogicalDevice(), frameBuffer, nullptr);
}

VulkanRHIGraphicsPipeline::VulkanRHIGraphicsPipeline(const RHIGraphicsPipelineInfo& info, VulkanRHIBackend& backend)
: RHIGraphicsPipeline(info)
{
    // 描述符 push constant
    std::vector<VkPushConstantRange> pushConstants;
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    for(const auto& pushConstant : info.rootSignature->GetInfo().GetPushConstants())
    {
        pushConstants.push_back(VulkanUtil::GetPushConstantInfo(pushConstant));
    }
    for(const auto& setInfo : ResourceCast(info.rootSignature)->GetSetInfos())
    {
        descriptorSetLayouts.push_back(setInfo.layout);
    }
    pipelineLayout = VulkanUtil::CreatePipelineLayout(backend.GetLogicalDevice(), descriptorSetLayouts, pushConstants);

    // 着色器
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    if(info.vertexShader)   shaderStages.push_back(ResourceCast(info.vertexShader)->GetShaderStageCreateInfo());
    if(info.geometryShader) shaderStages.push_back(ResourceCast(info.geometryShader)->GetShaderStageCreateInfo());
    if(info.fragmentShader) shaderStages.push_back(ResourceCast(info.fragmentShader)->GetShaderStageCreateInfo());

    // renderPass
    // 创建管线时需要指定一个renderPass，但是又没有一个严格的一一对应关系，使用时只需要renderPass彼此兼容
    // 又一处设计失败？
    uint32_t attachmentSize = 0;
    VulkanRenderPassAttachments renderPassAttachments = {};
    for(uint32_t i = 0; i < info.colorAttachmentFormats.size(); i++)
    {
        if(info.colorAttachmentFormats[i] == FORMAT_UKNOWN) break;
        attachmentSize++;
        
        renderPassAttachments.colorAttachments.push_back({
            .format = VulkanUtil::RHIFormatToVkFormat(info.colorAttachmentFormats[i]),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        });
    }
    renderPassAttachments.depthStencilAttachment = {
        .format = VulkanUtil::RHIFormatToVkFormat(info.depthStencilAttachmentFormat),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    };
    VkRenderPass renderPass = Backend()->FindOrCreateVkRenderPass(renderPassAttachments);


    // 光栅固定管线状态
    VkPipelineVertexInputStateCreateInfo vertexInputInfo    = GetInputStateCreateInfo(info.vertexInputState);
    VkPipelineInputAssemblyStateCreateInfo inputAssembly    = GetPipelineInputAssemblyStateCreateInfo(info.primitiveType);
    VkPipelineViewportStateCreateInfo viewportState         = GetPipelineViewportStateCreateInfo();
    VkPipelineRasterizationStateCreateInfo rasterizer       = GetPipelineRasterizationStateCreateInfo(info.rasterizerState);
    VkPipelineMultisampleStateCreateInfo multisampling      = GetPipelineMultisampleStateCreateInfo();
    VkPipelineColorBlendStateCreateInfo colorBlending       = GetPipelineColorBlendStateCreateInfo(info.blendState, attachmentSize);
    VkPipelineDepthStencilStateCreateInfo depthStencil      = GetPipelineDepthStencilStateCreateInfo(info.depthStencilState);
    VkPipelineDynamicStateCreateInfo dynamicState           = GetPipelineDynamicStateCreateInfo();

    GetDynamicInputStateCreateInfo(info.vertexInputState);


    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState; 
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  
    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.stageCount = (uint32_t)shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;  
	pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(backend.GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VK_NULL_HANDLE, &handle) != VK_SUCCESS) 
    {
        LOG_FATAL("Failed to create graphics pipeline!");
    }

}

void VulkanRHIGraphicsPipeline::Destroy()
{
    vkDestroyPipelineLayout(Backend()->GetLogicalDevice(), pipelineLayout, nullptr);
    vkDestroyPipeline(Backend()->GetLogicalDevice(), handle, nullptr); 
}

void VulkanRHIGraphicsPipeline::Bind(VkCommandBuffer commandBuffer)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, handle);

    vkCmdSetVertexInputEXT(commandBuffer,   // 加了一个动态绑定，暂时只为了不报错？
        (uint32_t)dynamicBindingDescriptions.size(),
        dynamicBindingDescriptions.data(),
        (uint32_t)dynamicAttributeDescriptions.size(),
        dynamicAttributeDescriptions.data());
}

void VulkanRHIComputePipeline::Bind(VkCommandBuffer commandBuffer)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, handle);
}

VkPipelineVertexInputStateCreateInfo VulkanRHIGraphicsPipeline::GetInputStateCreateInfo(const VertexInputStateInfo& vertexInputState) 
{
    for(const VertexElement& vertexElement : vertexInputState.vertexElements)
    {
        uint8_t binding = vertexElement.streamIndex;
        VkVertexInputAttributeDescription attributeDescription = {};
        attributeDescription.binding = binding;
        attributeDescription.location = vertexElement.attributeIndex;                                       // 对应layout location
        attributeDescription.format = VulkanUtil::RHIFormatToVkFormat(vertexElement.format);   // 属性格式
        attributeDescription.offset = vertexElement.offset;                                                 // 字段偏移
        attributeDescriptions.push_back(attributeDescription);
        
        while (bindingDescriptions.size() < vertexElement.streamIndex + 1) bindingDescriptions.push_back({});   // 下三项对所有同binding的vertexElement应该全部一致
        bindingDescriptions[binding].binding = binding;
        bindingDescriptions[binding].stride = vertexElement.stride;                                                                             //步长
        bindingDescriptions[binding].inputRate = vertexElement.useInstanceIndex ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;  //输入速率，逐顶点/逐实例
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)bindingDescriptions.size();
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    return vertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo VulkanRHIGraphicsPipeline::GetPipelineInputAssemblyStateCreateInfo(const PrimitiveType& primitiveType) 
{
    // 输入Assembly信息
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VulkanUtil::PrimitiveTypeToVk(primitiveType);      // 图元拓扑
    inputAssembly.primitiveRestartEnable = VK_FALSE;                            // 设为true，可以通过0xFFFF或者0xFFFFFFFF为特殊索引，分解_STRIP拓扑下的结构
    inputAssembly.flags = 0;

    return inputAssembly;
}

VkPipelineViewportStateCreateInfo VulkanRHIGraphicsPipeline::GetPipelineViewportStateCreateInfo() 
{
    // 视窗信息
    // VkViewport viewport = {};
    // viewport.x = 0.0f;
    // viewport.y = 0.0f;
    // viewport.width = (float)extent.width;
    // viewport.height = (float)extent.height;
    // viewport.minDepth = 0.0f;
    // viewport.maxDepth = 1.0f;

    // 裁剪矩形信息
    // VkRect2D scissor = {};
    // scissor.offset = { 0, 0 };
    // scissor.extent = extent;

    // 使用dynamic 不在这里创建
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = nullptr;
    viewportState.scissorCount = 1;
    viewportState.pScissors = nullptr;
    viewportState.flags = 0;

    return viewportState;
}

VkPipelineRasterizationStateCreateInfo VulkanRHIGraphicsPipeline::GetPipelineRasterizationStateCreateInfo(const RHIRasterizerStateInfo& rasterizerState) 
{
    // 光栅化信息
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = rasterizerState.depthClipMode == DEPTH_CLAMP ? VK_TRUE : VK_FALSE;            //对于超过远近裁剪平面的处理
    rasterizer.rasterizerDiscardEnable = VK_FALSE;                                                              //禁止图元传输
    rasterizer.polygonMode = VulkanUtil::FillModeToVk(rasterizerState.fillMode);                       //多边形的填充模式  
    rasterizer.lineWidth = 1.0f;                                                                                //填充模式为线框时的线宽度  
    rasterizer.cullMode = VulkanUtil::CullModeToVk(rasterizerState.cullMode);                          //裁剪模式
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;                                                     //面手性

    // rasterizer.depthBiasEnable = (  rasterizerState.depthBias > 0.0f || 
    //                                 rasterizerState.slopeScaleDepthBias > 0.0f) ? VK_TRUE : VK_FALSE;        //深度缓冲的bias
    rasterizer.depthBiasEnable = VK_TRUE;                                                                       //动态设置
    rasterizer.depthBiasConstantFactor = rasterizerState.depthBias; 
    rasterizer.depthBiasClamp = 0.0f; 
    rasterizer.depthBiasSlopeFactor = rasterizerState.slopeScaleDepthBias; 
    rasterizer.flags = 0;

    return rasterizer;
}

VkPipelineMultisampleStateCreateInfo VulkanRHIGraphicsPipeline::GetPipelineMultisampleStateCreateInfo() 
{
    // 多重采样信息
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;     // TODO 之后再支持
    multisampling.minSampleShading = 1.0f; 
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE; 
    multisampling.alphaToOneEnable = VK_FALSE; 
    multisampling.flags = 0;

    return multisampling;
}

VkPipelineColorBlendStateCreateInfo VulkanRHIGraphicsPipeline::GetPipelineColorBlendStateCreateInfo(const RHIBlendStateInfo& blendState, uint32_t size) 
{
    //混合信息
    for(uint32_t i = 0; i < size; i++)
    {
        auto& attachment = blendState.renderTargets[i];

        VkPipelineColorBlendAttachmentState attachmentState = {};
        attachmentState.blendEnable         = attachment.enable;
        attachmentState.colorBlendOp        = VulkanUtil::BlendOpToVk(attachment.colorBlendOp);
        attachmentState.srcColorBlendFactor = VulkanUtil::BlendFactorToVk(attachment.colorSrcBlend);
        attachmentState.dstColorBlendFactor = VulkanUtil::BlendFactorToVk(attachment.colorDstBlend);
        attachmentState.alphaBlendOp        = VulkanUtil::BlendOpToVk(attachment.alphaBlendOp);
        attachmentState.srcAlphaBlendFactor = VulkanUtil::BlendFactorToVk(attachment.alphaSrcBlend);
        attachmentState.dstAlphaBlendFactor = VulkanUtil::BlendFactorToVk(attachment.alphaDstBlend);
        attachmentState.colorWriteMask      = VulkanUtil::ColorWriteMaskToVk(attachment.colorWriteMask);
        
        blendStates.push_back(attachmentState);
    }
    
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; 
    colorBlending.attachmentCount = (uint32_t)blendStates.size();
    colorBlending.pAttachments = blendStates.data();
    colorBlending.blendConstants[0] = 0.0f; 
    colorBlending.blendConstants[1] = 0.0f; 
    colorBlending.blendConstants[2] = 0.0f; 
    colorBlending.blendConstants[3] = 0.0f; 

    return colorBlending;
}

VkPipelineDepthStencilStateCreateInfo VulkanRHIGraphicsPipeline::GetPipelineDepthStencilStateCreateInfo(const RHIDepthStencilStateInfo& depthStencilState) 
{ 
    //深度/模板缓冲信息
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = depthStencilState.enableDepthTest;                                               //深度测试
    depthStencil.depthWriteEnable = depthStencilState.enableDepthWrite;                                             //深度写入
    depthStencil.depthCompareOp = VulkanUtil::CompareFunctionToVk(depthStencilState.depthTest);    //深度比较方式
    depthStencil.depthBoundsTestEnable = VK_FALSE;              //边界检测
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f; 

    depthStencil.stencilTestEnable = VK_FALSE;                  //模板测试
    depthStencil.front = {}; 
    depthStencil.back = {}; 

    return depthStencil;
}

VkPipelineDynamicStateCreateInfo VulkanRHIGraphicsPipeline::GetPipelineDynamicStateCreateInfo() 
{
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = (uint32_t)dynamicStates.size();
    dynamicState.pDynamicStates = dynamicStates.data();

    return dynamicState;
}

void VulkanRHIGraphicsPipeline::GetDynamicInputStateCreateInfo(const VertexInputStateInfo& vertexInputState)
{
    // 跟静态的声明基本一样，但是填充的结构体不一样
    for(const VertexElement& vertexElement : vertexInputState.vertexElements)
    {
        uint8_t binding = vertexElement.streamIndex;
        VkVertexInputAttributeDescription2EXT dynamicAttributeDescription = {};
        dynamicAttributeDescription.sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
        dynamicAttributeDescription.binding = binding;
        dynamicAttributeDescription.location = vertexElement.attributeIndex;                                       
        dynamicAttributeDescription.format = VulkanUtil::RHIFormatToVkFormat(vertexElement.format);   
        dynamicAttributeDescription.offset = vertexElement.offset;                                                 
        dynamicAttributeDescriptions.push_back(dynamicAttributeDescription);
        
        while (dynamicBindingDescriptions.size() < vertexElement.streamIndex + 1) dynamicBindingDescriptions.push_back({});   
        dynamicBindingDescriptions[binding].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
        dynamicBindingDescriptions[binding].binding = binding;
        dynamicBindingDescriptions[binding].stride = vertexElement.stride;                                                                            
        dynamicBindingDescriptions[binding].divisor = 1;    // 这是啥？
        dynamicBindingDescriptions[binding].inputRate = vertexElement.useInstanceIndex ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;  
    }
}

VulkanRHIComputePipeline::VulkanRHIComputePipeline(const RHIComputePipelineInfo& info, VulkanRHIBackend& backend)
: RHIComputePipeline(info)
{
    // 描述符 push constant
    std::vector<VkPushConstantRange> pushConstants;
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    for(const auto& pushConstant : info.rootSignature->GetInfo().GetPushConstants())
    {
        pushConstants.push_back(VulkanUtil::GetPushConstantInfo(pushConstant));
    }
    for(const auto& setInfo : ResourceCast(info.rootSignature)->GetSetInfos())
    {
        descriptorSetLayouts.push_back(setInfo.layout);
    }
    pipelineLayout = VulkanUtil::CreatePipelineLayout(backend.GetLogicalDevice(), descriptorSetLayouts, pushConstants);

    // 着色器
    VkPipelineShaderStageCreateInfo shaderStage = ResourceCast(info.computeShader)->GetShaderStageCreateInfo();



    VkComputePipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.stage = shaderStage;
    pipelineInfo.layout = pipelineLayout;
    
    if (vkCreateComputePipelines(backend.GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VK_NULL_HANDLE, &handle) != VK_SUCCESS) 
    {
        LOG_FATAL("Failed to create compute pipeline!");
    }
}

void VulkanRHIComputePipeline::Destroy()
{
    vkDestroyPipelineLayout(Backend()->GetLogicalDevice(), pipelineLayout, nullptr);
    vkDestroyPipeline(Backend()->GetLogicalDevice(), handle, nullptr); 
}

//同步 ////////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanRHIFence::VulkanRHIFence(bool signaled, VulkanRHIBackend& backend)
{
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;  

    vkCreateFence(backend.GetLogicalDevice(), &fenceInfo, nullptr, &handle);
}

void VulkanRHIFence::Wait()
{
    vkWaitForFences(Backend()->GetLogicalDevice(), 1, &handle, VK_TRUE, UINT64_MAX);    //TODO 设置超时时间
    vkResetFences(Backend()->GetLogicalDevice(), 1, &handle);
}

void VulkanRHIFence::Destroy()
{
    vkDestroyFence(Backend()->GetLogicalDevice(), handle, nullptr);
}

VulkanRHISemaphore::VulkanRHISemaphore(VulkanRHIBackend& backend)
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    vkCreateSemaphore(backend.GetLogicalDevice(), &semaphoreInfo, nullptr, &handle);
}

void VulkanRHISemaphore::Destroy()
{
    vkDestroySemaphore(Backend()->GetLogicalDevice(), handle, nullptr);
}

