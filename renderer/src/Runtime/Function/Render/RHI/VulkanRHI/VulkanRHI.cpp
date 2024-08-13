#include "VulkanRHI.h"
#include "VulkanRHIResource.h"
#include "VulkanUtil.h"
#include "Function/Render/RHI/RHIResource.h"
#include "Function/Render/RHI/RHIStructs.h"
#include "Function/Render/RHI/RHI.h"
#include "Platform/HAL/PlatformProcess.h"
#include "Core/Log/Log.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

VulkanRHIBackend::VulkanRHIBackend(const RHIBackendInfo& info) 
: RHIBackend(info)
, sync(PlatformProcess::CreateCriticalSection())
{
    CreateInstance();   
    CreatePhysicalDevice();
    CreateLogicalDevice();
    CreateQueues();
    CreateMemoryAllocator();
    CreateDescriptorPool();
    CreateImmediateCommand();
}

void VulkanRHIBackend::Destroy()
{
    for(auto& list : queues)
    {
        for (auto& queue : list) queue->WaitIdle();
    } 
    
    RHIBackend::Destroy();

    renderPassPool.Clear();
    frameBufferPool.Clear();
    vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);

    vmaDestroyAllocator(memoryAllocator);
    vkDestroyDevice(logicalDevice, nullptr);
    vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    vkDestroyInstance(instance, nullptr);
}

//基本资源 ////////////////////////////////////////////////////////////////////////////////////////////////////////

RHIQueueRef VulkanRHIBackend::GetQueue(const RHIQueueInfo& info) 
{
    return queues[info.type][info.index];
}

RHISurfaceRef VulkanRHIBackend::CreateSurface(GLFWwindow* window)
{
    RHISurfaceRef surface = std::make_shared<VulkanRHISurface>(window, *this);
    RegisterResource(surface);

    return surface;
}

RHISwapchainRef VulkanRHIBackend::CreateSwapChain(const RHISwapchainInfo& info)
{
    RHISwapchainRef swapchain = std::make_shared<VulkanRHISwapchain>(info, *this);
    RegisterResource(swapchain);

    return swapchain;
}

RHICommandPoolRef VulkanRHIBackend::CreateCommandPool(const RHICommandPoolInfo& info) 
{
    RHICommandPoolRef commandPool = std::make_shared<VulkanRHICommandPool>(info, *this);
    RegisterResource(commandPool);

    return commandPool;
}

RHICommandContextRef VulkanRHIBackend::CreateCommandContext(RHICommandPool* pool)
{
    RHICommandContextRef commandContext = std::make_shared<VulkanRHICommandContext>(pool, *this);
    RegisterResource(commandContext);

    return commandContext;
}

//缓冲，纹理 ////////////////////////////////////////////////////////////////////////////////////////////////////////

RHIBufferRef VulkanRHIBackend::CreateBuffer(const RHIBufferInfo& info) 
{ 
    RHIBufferRef buffer = std::make_shared<VulkanRHIBuffer>(info, *this);
    RegisterResource(buffer);

    return buffer;
}

RHITextureRef VulkanRHIBackend::CreateTexture(const RHITextureInfo& info) 
{ 
    RHITextureRef texture = std::make_shared<VulkanRHITexture>(info, *this);
    RegisterResource(texture);

    return texture;
}

RHITextureViewRef VulkanRHIBackend::CreateTextureView(const RHITextureViewInfo& info)
{
    RHITextureViewRef textureView = std::make_shared<VulkanRHITextureView>(info, *this);
    RegisterResource(textureView);

    return textureView;
}

RHISamplerRef VulkanRHIBackend::CreateSampler(const RHISamplerInfo& info)
{
    RHISamplerRef sampler = std::make_shared<VulkanRHISampler>(info, *this);
    RegisterResource(sampler);

    return sampler;
}

//着色器 ////////////////////////////////////////////////////////////////////////////////////////////////////////

RHIShaderRef VulkanRHIBackend::CreateShader(const RHIShaderInfo& info) 
{ 
    RHIShaderRef shader = std::make_shared<VulkanRHIShader>(info, *this);
    RegisterResource(shader);

    return shader;
}

RHIRootSignatureRef VulkanRHIBackend::CreateRootSignature(const RHIRootSignatureInfo& info) 
{ 
    RHIRootSignatureRef rootSignature = std::make_shared<VulkanRHIRootSignature>(info, *this);
    RegisterResource(rootSignature);

    return rootSignature;
}

//管线状态 ////////////////////////////////////////////////////////////////////////////////////////////////////////

RHIRenderPassRef VulkanRHIBackend::CreateRenderPass(const RHIRenderPassInfo& info)
{
    RHIRenderPassRef renderPass = std::make_shared<VulkanRHIRenderPass>(info, *this);
    RegisterResource(renderPass);

    return renderPass;
}

RHIGraphicsPipelineRef VulkanRHIBackend::CreateGraphicsPipeline(const RHIGraphicsPipelineInfo& info) 
{ 
    RHIGraphicsPipelineRef graphicsPipeline = std::make_shared<VulkanRHIGraphicsPipeline>(info, *this);
    RegisterResource(graphicsPipeline);

    return graphicsPipeline;
}

RHIComputePipelineRef VulkanRHIBackend::CreateComputePipeline(const RHIComputePipelineInfo& info) 
{ 
    RHIComputePipelineRef computePipeline = std::make_shared<VulkanRHIComputePipeline>(info, *this);
    RegisterResource(computePipeline);

    return computePipeline;
}

RHIRayTracingPipelineRef VulkanRHIBackend::CreateRayTracingPipeline(const RHIRayTracingPipelineInfo& info) { return nullptr; }

//同步 ////////////////////////////////////////////////////////////////////////////////////////////////////////

RHIFenceRef VulkanRHIBackend::CreateFence(bool signaled) 
{
    RHIFenceRef fence = std::make_shared<VulkanRHIFence>(signaled, *this);
    RegisterResource(fence);

    return fence;
}

RHISemaphoreRef VulkanRHIBackend::CreateSemaphore()
{
    RHISemaphoreRef semaphore = std::make_shared<VulkanRHISemaphore>(*this);
    RegisterResource(semaphore);

    return semaphore;
}

//立即模式的命令接口 ////////////////////////////////////////////////////////////////////////////////////////////////////////

RHIImmediateCommandRef VulkanRHIBackend::GetImmediateCommand() 
{
    return immediateCommand;
}

void VulkanRHIBackend::CreateInstance()
{
    //使用volk库来做vulkan函数引入，先初始化
    if (volkInitialize() != VK_SUCCESS) {
        LOG_FATAL("Volk initialize failed!");
        return;
    }

    //instance的layer信息
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        availableLayers = std::vector<VkLayerProperties>(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (auto& layer : availableLayers)
        {
            std::cout << layer.layerName << " " << layer.description << " " << layer.specVersion << " " << layer.implementationVersion << std::endl;
        }
        std::cout << " " << std::endl;
    }

    // 初始化VkApplicationInfo
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Toy Render Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VULKAN_VERSION;    //使用的Vulkan API版本号

    // 初始化VkInstanceCreateInfo，全局信息？
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto validationLayers = VulkanUtil::GetDebugMessengerCreateInfo();
    std::vector<const char*> layers;
    if (backendInfo.enableDebug)   // 验证层,不做检查了
    {             
        for (auto& layer : INSTANCE_LAYERS) layers.push_back(layer);

        createInfo.enabledLayerCount = (uint32_t)layers.size();
        createInfo.ppEnabledLayerNames = layers.data();

        // debug扩展信息
        createInfo.pNext = &validationLayers;
    }

    // 扩展支持信息，不做检查了
    auto instanceExtentions = VulkanUtil::GetRequiredInstanceExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtentions.size());
    createInfo.ppEnabledExtensionNames = instanceExtentions.data();

    // 创建Vulkan实例
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) 
    {
        LOG_FATAL("Failed to create instance!");
    }

    //volk初始化
    volkLoadInstance(instance); 
    
    //创建DebugMessager
    if (backendInfo.enableDebug)
    {
        VkDebugUtilsMessengerCreateInfoEXT info = VulkanUtil::GetDebugMessengerCreateInfo();
        if (vkCreateDebugUtilsMessengerEXT(instance, &info, nullptr, &debugMessenger) != VK_SUCCESS) 
        {
            LOG_FATAL("Failed to set up debug messenger!");
        }
    }
}

void VulkanRHIBackend::CreatePhysicalDevice()
{
    // 选择支持的硬件
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (auto& device : devices) {
        vkGetPhysicalDeviceProperties(device, &properties);
        for (auto target : TARGET_DEVICES)
        {
            std::string name = std::string(properties.deviceName);
            std::transform(name.begin(), name.end(), name.begin(), std::toupper);
            if (name.find(target) != std::string::npos)    //直接用名称查找了，不检查各种要求
            {
                //基本信息
                {
                    std::cout << " " << std::endl;
                    std::cout << properties.deviceName << std::endl;

                    std::cout << "framebufferColorSampleCounts : " << properties.limits.framebufferColorSampleCounts << std::endl;
                    std::cout << "framebufferDepthSampleCounts : " << properties.limits.framebufferDepthSampleCounts << std::endl;
                    std::cout << "framebufferStencilSampleCounts : " << properties.limits.framebufferStencilSampleCounts << std::endl;
                    std::cout << "maxColorAttachments : " << properties.limits.maxColorAttachments << std::endl;
                    std::cout << "maxDescriptorSetInputAttachments : " << properties.limits.maxDescriptorSetInputAttachments << std::endl;
                    std::cout << "maxDescriptorSetUniformBuffers : " << properties.limits.maxDescriptorSetUniformBuffers << std::endl;
                    std::cout << "maxFramebufferLayers : " << properties.limits.maxFramebufferLayers << std::endl;
                    std::cout << "maxPushConstantsSize : " << properties.limits.maxPushConstantsSize << std::endl;
                    std::cout << "maxBoundDescriptorSets : " << properties.limits.maxBoundDescriptorSets << std::endl;
                    std::cout << "maxComputeWorkGroupInvocations : " << properties.limits.maxComputeWorkGroupInvocations << std::endl;
                    std::cout << "minStorageBufferOffsetAlignment : " << properties.limits.minStorageBufferOffsetAlignment << std::endl;
                    
                }

                //初始化存储物理设备支持的各种属性，特性，扩展，内存特性，队列信息
                //vkGetPhysicalDeviceFeatures2(device, &fetures2);
                vkGetPhysicalDeviceFeatures(device, &features);
                vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);
                //for (int i = 0; i < memoryProperties.memoryTypeCount; i++)
                //{
                //    std::cout << "\t" << memoryProperties.memoryTypes[i].propertyFlags << std::endl;
                //}

                uint32_t queueFamilyCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
                queueFamilyProperties.resize(queueFamilyCount);
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

                uint32_t extCount = 0;
                vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);
                std::vector<VkExtensionProperties> extensions(extCount);
                if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
                {
                    std::cout << " " << std::endl;
                    for (auto& ext : extensions)
                    {
                        supportedExtensions.push_back(ext.extensionName);
                        std::cout << ext.extensionName << std::endl;
                    }
                }

                if(backendInfo.enableRayTracing)
                {
                    //获取光追管线特性
                    rayTracingPipelineProperties = {};
                    rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

                    VkPhysicalDeviceProperties2 deviceProperties2 = {};
                    deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
                    deviceProperties2.pNext = &rayTracingPipelineProperties;
                    vkGetPhysicalDeviceProperties2(device, &deviceProperties2);
                }

                physicalDevice = device;
                return;
            }
        }
    }
    
    LOG_FATAL("Target device not found!");
}

void VulkanRHIBackend::CreateLogicalDevice()
{
    // 获取队列族信息
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    queueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    for(auto& index : queueIndices) index = -1;

    std::cout << " " << std::endl;
    for (auto& queueFamily : queueFamilyProperties)
    {
        std::cout << "Queue count: " << queueFamily.queueCount << std::endl;
        std::cout << "Queue flags: " << VulkanUtil::QueueFlagsToString(queueFamily.queueFlags) << std::endl;
    }

    std::vector<uint32_t> allocatedCounts(queueFamilyProperties.size());
    for (int i = 0; i < queueFamilyProperties.size(); i++) 
    {
        auto& queueFamily = queueFamilyProperties[i];
        uint32_t queueCount = queueFamily.queueCount;   // 多个不同属性的队列可以从同一个队列族分配，只要数量够就行，队列族不只支持一种属性
                                                        // 背包问题 XD
        if (queueCount > MAX_QUEUE_CNT &&
            queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT &&
            queueIndices[QUEUE_TYPE_GRAPHICS] < 0)
        {
            queueIndices[QUEUE_TYPE_GRAPHICS] = i;  
            queueCount -= MAX_QUEUE_CNT;
        }    
        if (queueCount > MAX_QUEUE_CNT &&
            queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT &&
            queueIndices[QUEUE_TYPE_COMPUTE] < 0)
        {
            queueIndices[QUEUE_TYPE_COMPUTE] = i; 
            queueCount -= MAX_QUEUE_CNT;
        }
        if (queueCount > MAX_QUEUE_CNT &&
            queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT &&
            queueIndices[QUEUE_TYPE_TRANSFER] < 0)
        {
            queueIndices[QUEUE_TYPE_TRANSFER] = i; 
            queueCount -= MAX_QUEUE_CNT;
        }

        allocatedCounts[i] = queueFamily.queueCount - queueCount;

        // 好像graphics queue就支持了？不需要单独处理？
        // // 窗口支持
        // VkBool32 presentSupport = false;
        // vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        // if (queueFamily.queueCount > 0 && presentSupport && presentFamily < 0 && graphicsFamily != i) { //强行使用不同的queue
        //     presentFamily = i;
        // }
    }
    for(int i = 0; i < QUEUE_TYPE_MAX_ENUM; i++) if(queueIndices[i] < 0) LOG_FATAL("Fail to allocate queue!");





    // 创建设备请求信息
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    //队列信息
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for(int i = 0; i < allocatedCounts.size(); i++)
    {
        if(allocatedCounts[i] == 0) continue;

        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = i;                               //队列族
        queueCreateInfo.queueCount = allocatedCounts[i];                    //队列数目，多个队列可以支持并行异步计算
        queueCreateInfo.pQueuePriorities = QUEUE_PRIORITIES;                //优先级
        queueCreateInfos.push_back(queueCreateInfo);
    }
    createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();   
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    //扩展信息
    std::vector<const char*> deviceExtentions;
    for (auto extention : DEVICE_EXTENTIONS) deviceExtentions.push_back(extention);
    if (backendInfo.enableRayTracing) { for (auto extention : RAY_TRACING_DEVICE_EXTENTIONS) deviceExtentions.push_back(extention); }
    createInfo.enabledExtensionCount = (uint32_t)deviceExtentions.size(); 
    createInfo.ppEnabledExtensionNames = deviceExtentions.data();

    //验证层信息
    std::vector<const char*> devicelayers;
    if (backendInfo.enableDebug)
    {           
        for (auto layer : DEVICE_LAYERS) devicelayers.push_back(layer);

        createInfo.enabledLayerCount = (uint32_t)devicelayers.size(); 
        createInfo.ppEnabledLayerNames = devicelayers.data();
    }

    // 特性支持
    VkPhysicalDeviceFeatures deviceFeatures = {};       //设备特性信息
    deviceFeatures.samplerAnisotropy = VK_TRUE;         //请求各向异性采样支持
    deviceFeatures.geometryShader = VK_TRUE;            //几何着色器支持
    deviceFeatures.tessellationShader = VK_TRUE;        //曲面细分着色器支持
    deviceFeatures.pipelineStatisticsQuery = VK_TRUE;   //管线统计查询支持
    deviceFeatures.fillModeNonSolid = VK_TRUE;          //线框模式
    deviceFeatures.multiDrawIndirect = VK_TRUE;         //多重间接绘制
    deviceFeatures.independentBlend = VK_TRUE;          //MRT单独设置每个混合状态
    createInfo.pEnabledFeatures = &deviceFeatures;

    VkPhysicalDeviceVulkan12Features vulkan12Features{};                                        //1.2版本的其他支持
    vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    vulkan12Features.samplerFilterMinmax = VK_TRUE;                                             //采样器的过滤模式，用于Hiz
                                                                                                //bindless支持，描述符索引
    vulkan12Features.runtimeDescriptorArray = VK_TRUE;                                          //SPIR-V 中使用动态数组
    vulkan12Features.descriptorBindingVariableDescriptorCount = VK_TRUE;                        //DescriptorSet Layout 的 Binding 中使用可变大小的 AoD
    vulkan12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;                       //SPIR-V 中通过 nonuniformEXT Decoration 用于非 Uniform 变量下标索引资源数组
    vulkan12Features.descriptorBindingPartiallyBound = VK_TRUE;
    vulkan12Features.descriptorIndexing = VK_TRUE;
    createInfo.pNext = &vulkan12Features;

    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT dynamicVertexInputFeatures = {};         //动态顶点输入描述
    dynamicVertexInputFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT;
    dynamicVertexInputFeatures.vertexInputDynamicState = VK_TRUE;
    vulkan12Features.pNext = &dynamicVertexInputFeatures;

    //这个扩展已经在1.2提为core特性，可以放在VkPhysicalDeviceVulkan12Features里声明   //最开始是 VkPhysicalDeviceDescriptorIndexingFeaturesEXT
    //VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};                          //bindless支持，描述符索引
    //indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    //indexingFeatures.runtimeDescriptorArray = VK_TRUE;                      //SPIR-V 中使用动态数组
    //indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;    //DescriptorSet Layout 的 Binding 中使用可变大小的 AoD
    //indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;   //SPIR-V 中通过 nonuniformEXT Decoration 用于非 Uniform 变量下标索引资源数组
    //indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
    //rayTracingPipelineFeatures.pNext = &indexingFeatures;

    //VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT divisorFeatures = {};               //请求逐实例参数支持
    //divisorFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT;
    //divisorFeatures.vertexAttributeInstanceRateDivisor = VK_TRUE;
    //divisorFeatures.vertexAttributeInstanceRateZeroDivisor = VK_TRUE;
    //createInfo.pNext = &divisorFeatures;    //申请新特性（API1.0之后的）支持，链式申请


    VkPhysicalDeviceBufferDeviceAddressFeaturesEXT deviceAddressfeture = {};                //请求内存地址信息
    deviceAddressfeture.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT;
    deviceAddressfeture.bufferDeviceAddress = VK_TRUE;

    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = {};    //rt加速结构
    accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    accelerationStructureFeatures.accelerationStructure = true;

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures = {};          //rt管线
    rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    rayTracingPipelineFeatures.rayTracingPipeline = true;

    VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures = {};                              //rt查询
    rayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
    rayQueryFeatures.rayQuery = VK_TRUE;

    if(backendInfo.enableRayTracing)
    {
        dynamicVertexInputFeatures.pNext = &deviceAddressfeture;    
        deviceAddressfeture.pNext = &accelerationStructureFeatures;
        accelerationStructureFeatures.pNext = &rayTracingPipelineFeatures;
        rayTracingPipelineFeatures.pNext = &rayQueryFeatures;
    }

    VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice);
    if (result != VK_SUCCESS) 
    {
        LOG_FATAL("Failed to create logical device! [%d]", result);
    }

    volkLoadDevice(logicalDevice);  //volk
    //volkLoadDeviceTable(struct VolkDeviceTable* table, VkDevice device);    //volk
}

void VulkanRHIBackend::CreateQueues()
{
    std::vector<uint32_t> offsets(queueFamilyProperties.size(), {0}) ;
    for (uint32_t i = 0; i < QUEUE_TYPE_MAX_ENUM; i++)
    {
        for(uint32_t j = 0; j < MAX_QUEUE_CNT; j++)
        {
            VkQueue queue;
            vkGetDeviceQueue(logicalDevice, queueIndices[i], offsets[i], &queue);

            RHIQueueInfo info = 
            {
                .type = (QueueType)i,
                .index = j,
            };
            queues[i][j] = std::make_shared<VulkanRHIQueue>(info, queue, queueIndices[i]);
            RegisterResource(queues[i][j]);

            offsets[i]++;
        }
    }
}

void VulkanRHIBackend::CreateMemoryAllocator()
{
    // volk集成vma: https://zhuanlan.zhihu.com/p/634912614 
    // vma官方文档: https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/index.html
    // 对照该表，不同vulkan版本有不同的绑定要求: https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_vulkan_functions.html

    VmaVulkanFunctions vulkanFunctions{};
    vulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
    vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    vulkanFunctions.vkAllocateMemory = vkAllocateMemory;
    vulkanFunctions.vkFreeMemory = vkFreeMemory;
    vulkanFunctions.vkMapMemory = vkMapMemory;
    vulkanFunctions.vkUnmapMemory = vkUnmapMemory;
    vulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
    vulkanFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
    vulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
    vulkanFunctions.vkBindImageMemory = vkBindImageMemory;
    vulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;  
    vulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
    vulkanFunctions.vkCreateBuffer = vkCreateBuffer;
    vulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
    vulkanFunctions.vkCreateImage = vkCreateImage;
    vulkanFunctions.vkDestroyImage = vkDestroyImage;
    vulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;
    // vulkanFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
    // vulkanFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
    // vulkanFunctions.vkBindBufferMemory2KHR = vkBindBufferMemory2KHR;
    // vulkanFunctions.vkBindImageMemory2KHR = vkBindImageMemory2KHR;
    // vulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR;
    vulkanFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2;
    vulkanFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2;
    vulkanFunctions.vkBindBufferMemory2KHR = vkBindBufferMemory2;
    vulkanFunctions.vkBindImageMemory2KHR = vkBindImageMemory2;
    vulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;
    vulkanFunctions.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;  
    vulkanFunctions.vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.vulkanApiVersion = VULKAN_VERSION;
    allocatorCreateInfo.physicalDevice = physicalDevice;
    allocatorCreateInfo.device = logicalDevice;
    allocatorCreateInfo.instance = instance;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    vmaCreateAllocator(&allocatorCreateInfo, &memoryAllocator);
}

void VulkanRHIBackend::CreateDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 4096 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4096 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 4096 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 4096 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 4096 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4096 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4096 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 4096 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 4096 },
    };

    //描述符池信息
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
    poolInfo.pPoolSizes = descriptorPoolSizes.data();
    poolInfo.maxSets = 8192;               //指定最大描述符集合数
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;   //TODO 使得描述符可以实时更新

    if (vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) 
    {
        LOG_FATAL("Failed to create descriptor pool!");
    }
}

void VulkanRHIBackend::CreateImmediateCommand()
{
    immediateCommand = std::make_shared<VulkanRHIImmediateCommand>(*this);
    RegisterResource(immediateCommand);
}

VkRenderPass VulkanRHIBackend::CreateVkRenderPass(const VulkanRenderPassAttachments& info)
{
    bool hasDepth = (info.depthStencilAttachment.format != VK_FORMAT_UNDEFINED);
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentReference> colorReferences;
    VkAttachmentReference depthReference = {};

    for(const VkAttachmentDescription& attachment : info.colorAttachments)  attachments.push_back(attachment);
    for(VkAttachmentDescription& attachment : attachments) 
    {
        attachment.stencilLoadOp = attachment.loadOp;   // 写死
        attachment.stencilStoreOp = attachment.storeOp;
        attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    if(hasDepth) 
    {
        VkAttachmentDescription attachment = info.depthStencilAttachment;
        attachment.stencilLoadOp = attachment.loadOp;   // 写死
        attachment.stencilStoreOp = attachment.storeOp;
        attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachments.push_back(attachment);
    }

    for(uint32_t i = 0; i < attachments.size() - 1; i++) colorReferences.push_back({ i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	VkAttachmentReference depthAttachmentReference = { (uint32_t)attachments.size() - 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    // 不支持subpass了 艹
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = (uint32_t)colorReferences.size();
    subpass.pColorAttachments = colorReferences.data();
    subpass.pDepthStencilAttachment = hasDepth ? &depthAttachmentReference : nullptr;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;
    subpass.pResolveAttachments = nullptr;

    // 屏障在外面显式的添加 不在pass里加了？
    // std::vector<VkSubpassDependency> subpassDependencies(2);
    // subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    // subpassDependencies[0].dstSubpass = 0;
    // subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    // subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    // subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    // subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    // subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    // subpassDependencies[1].srcSubpass = 0;
    // subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    // subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    // subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    // subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    // subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 0;
    renderPassInfo.pDependencies = nullptr;

    VkRenderPass renderPass;
    if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) 
    {
        LOG_FATAL("Failed to create render pass!");
    }

    // {
    //     ScopeLock lock(sync);
    //     renderPassMap.insert({info, renderPass});
    // }
    return renderPass;
}

VkFramebuffer VulkanRHIBackend::CreateVkFramebuffer(const VkFramebufferCreateInfo& info)
{
    VkFramebuffer frameBuffer;
    if (vkCreateFramebuffer(logicalDevice, &info, nullptr, &frameBuffer) != VK_SUCCESS) 
    {
        LOG_FATAL("Failed to create framebuffer!");
    }

    return frameBuffer;
}







void TextureBarrier(VkCommandBuffer commandBuffer, const RHITextureBarrier& barrier)
{
    TextureSubresourceRange range = barrier.subresource;
    if (range.aspect == TEXTURE_ASPECT_NONE) range = barrier.texture->GetDefaultSubresourceRange();

    VkAccessFlags srcAccessMask = VulkanUtil::ResourceStateToAccessFlags(barrier.srcState);
    VkAccessFlags dstAccessMask = VulkanUtil::ResourceStateToAccessFlags(barrier.dstState);
    VkPipelineStageFlags srcStage = VulkanUtil::AccessFlagsToPipelineStageFlags(srcAccessMask);
    VkPipelineStageFlags dstStage = VulkanUtil::AccessFlagsToPipelineStageFlags(dstAccessMask);

    VkImageMemoryBarrier memoryBarrier = {};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memoryBarrier.oldLayout = VulkanUtil::ResourceStateToImageLayout(barrier.srcState);
    memoryBarrier.newLayout = VulkanUtil::ResourceStateToImageLayout(barrier.dstState);
    memoryBarrier.image = ResourceCast(barrier.texture)->GetHandle();
    memoryBarrier.subresourceRange = VulkanUtil::SubresourceToVk(range);
    memoryBarrier.srcAccessMask = srcAccessMask;
    memoryBarrier.dstAccessMask = dstAccessMask;

    vkCmdPipelineBarrier(
        commandBuffer,
        srcStage, dstStage, 0,
        0, nullptr,
        0, nullptr,
        1, &memoryBarrier);
}

void BufferBarrier(VkCommandBuffer commandBuffer, const RHIBufferBarrier& barrier)
{
    VkAccessFlags srcAccessMask = VulkanUtil::ResourceStateToAccessFlags(barrier.srcState);
    VkAccessFlags dstAccessMask = VulkanUtil::ResourceStateToAccessFlags(barrier.dstState);
    VkPipelineStageFlags srcStage = VulkanUtil::AccessFlagsToPipelineStageFlags(srcAccessMask);
    VkPipelineStageFlags dstStage = VulkanUtil::AccessFlagsToPipelineStageFlags(dstAccessMask);

    VkBufferMemoryBarrier memoryBarrier = {};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memoryBarrier.srcAccessMask = srcAccessMask;
    memoryBarrier.dstAccessMask = dstAccessMask;
    memoryBarrier.buffer = ResourceCast(barrier.buffer)->GetHandle();
    memoryBarrier.offset = barrier.offset;               // TODO
    memoryBarrier.size = barrier.size == 0 ? VK_WHOLE_SIZE : barrier.size;

    vkCmdPipelineBarrier(
        commandBuffer,
        srcStage, dstStage, 0,
        0, nullptr,
        1, &memoryBarrier,
        0, nullptr);
}

void CopyTextureToBuffer(VkCommandBuffer commandBuffer, RHITextureRef src, TextureSubresourceLayers srcSubresource, RHIBufferRef dst, uint64_t dstOffset)
{
    VkBufferImageCopy copy = {};
    copy.bufferOffset = dstOffset;
    copy.bufferRowLength = 0;       // TODO
    copy.bufferImageHeight = 0;
    copy.imageSubresource = VulkanUtil::SubresourceToVk(srcSubresource);
    copy.imageOffset = {0, 0, 0};
    copy.imageExtent = VulkanUtil::ExtentToVk(src->MipExtent(srcSubresource.mipLevel));

    vkCmdCopyImageToBuffer(commandBuffer,
    ResourceCast(src)->GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    ResourceCast(dst)->GetHandle(),
    1, &copy);
}

void CopyBufferToTexture(VkCommandBuffer commandBuffer, RHIBufferRef src, uint64_t srcOffset, RHITextureRef dst, TextureSubresourceLayers dstSubresource)
{
    VkBufferImageCopy copy = {};
    copy.bufferOffset = srcOffset;
    copy.bufferRowLength = 0;       // TODO
    copy.bufferImageHeight = 0;
    copy.imageSubresource = VulkanUtil::SubresourceToVk(dstSubresource);
    copy.imageOffset = {0, 0, 0};
    copy.imageExtent = VulkanUtil::ExtentToVk(dst->MipExtent(dstSubresource.mipLevel));

    vkCmdCopyBufferToImage(commandBuffer, 
    ResourceCast(src)->GetHandle(),
    ResourceCast(dst)->GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
    1, &copy);
}

void CopyBuffer(VkCommandBuffer commandBuffer, RHIBufferRef src, uint64_t srcOffset, RHIBufferRef dst, uint64_t dstOffset, uint64_t size)
{
    VkBufferCopy copy = {};
    copy.srcOffset = srcOffset;
    copy.dstOffset = dstOffset;
    copy.size = size;

    vkCmdCopyBuffer(commandBuffer,
    ResourceCast(src)->GetHandle(),
    ResourceCast(dst)->GetHandle(),
    1, &copy);
}

void CopyTexture(VkCommandBuffer commandBuffer, RHITextureRef src, TextureSubresourceLayers srcSubresource, RHITextureRef dst, TextureSubresourceLayers dstSubresource)
{
    VkImageCopy imageCopy = {};
    imageCopy.srcOffset = {0, 0, 0};    // TODO ?
    imageCopy.dstOffset = {0, 0, 0};
    imageCopy.srcSubresource = VulkanUtil::SubresourceToVk(srcSubresource);
    imageCopy.dstSubresource = VulkanUtil::SubresourceToVk(dstSubresource);
    imageCopy.extent = VulkanUtil::ExtentToVk(src->MipExtent(srcSubresource.mipLevel));

    vkCmdCopyImage(commandBuffer,
        ResourceCast(src)->GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        ResourceCast(dst)->GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &imageCopy);
}

// 假定处于正确的src在src状态，dst在dst状态
void BlitTexture(   VkCommandBuffer commandBuffer, 
                    RHITextureRef src, RHITextureRef dst, 
                    TextureSubresourceLayers srcSubresource, TextureSubresourceLayers dstSubresource, 
                    FilterType filter)
{
    VkImageSubresourceLayers srcLayer = VulkanUtil::SubresourceToVk(srcSubresource);
    VkImageSubresourceLayers dstLayer = VulkanUtil::SubresourceToVk(dstSubresource);

    uint32_t srcMip = srcSubresource.mipLevel;
    uint32_t dstMip = dstSubresource.mipLevel;

    VkImageBlit blit = {};
    blit.srcOffsets[0] = {0, 0, 0}; //TODO offset
    blit.srcOffsets[1] = {  (int32_t)(src->GetInfo().extent.width / pow(2, srcMip)),
                            (int32_t)(src->GetInfo().extent.height / pow(2, srcMip)), 1};
    blit.srcSubresource = srcLayer;

    blit.dstOffsets[0] = {0, 0, 0};
    blit.dstOffsets[1] = {  (int32_t)(dst->GetInfo().extent.width / pow(2, dstMip)),
                            (int32_t)(dst->GetInfo().extent.height / pow(2, dstMip)), 1};
    blit.dstSubresource = dstLayer;

    vkCmdBlitImage(commandBuffer,
        ResourceCast(src)->GetHandle(),
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        ResourceCast(dst)->GetHandle(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &blit,
        VulkanUtil::FilterTypeToVk(filter));
}

// 假定起始时各层均为transfer src状态，结束时也为src状态
void GenerateMips(VkCommandBuffer commandBuffer, RHITextureRef src)
{
    //总计生成的mip层数
    uint32_t mipLevels = src->GetInfo().mipLevels;   

    VkImageSubresourceRange transition = {};
    transition.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    transition.baseMipLevel = 0;
    transition.levelCount = 1;
    transition.baseArrayLayer = 0;
    transition.layerCount = 1;

    for(uint32_t i = 0; i < src->GetInfo().arrayLayers; i++)
    {
        transition.baseMipLevel = 0;
        transition.baseArrayLayer = i;

        // 先将后面的层全设置到dst
        TextureBarrier(commandBuffer, 
        {src, 
        RESOURCE_STATE_TRANSFER_SRC, RESOURCE_STATE_TRANSFER_DST, 
                {TEXTURE_ASPECT_COLOR, 1, mipLevels - 1, transition.baseArrayLayer, transition.layerCount}});

        //循环生成各级mip，并将对应层级转到srcLayout
        for (uint32_t i = 1; i < mipLevels; i++)    //总共mipLevels级，只需要mipLevels-1次blit
        {   
            BlitTexture(
                commandBuffer, 
                src, 
                src, 
                { transition.aspectMask, transition.baseMipLevel, transition.baseArrayLayer, transition.layerCount },
                { transition.aspectMask, transition.baseMipLevel + 1, transition.baseArrayLayer, transition.layerCount },
                FILTER_TYPE_LINEAR);

            // 将生成后的层级设置到src
            TextureBarrier(commandBuffer, 
            {src, 
            RESOURCE_STATE_TRANSFER_DST, RESOURCE_STATE_TRANSFER_SRC, 
                    {TEXTURE_ASPECT_COLOR, transition.baseMipLevel + 1, 1, transition.baseArrayLayer, transition.layerCount}});

            transition.baseMipLevel++;
        }
    }
}














VulkanRHICommandContext::VulkanRHICommandContext(RHICommandPool* pool, const VulkanRHIBackend& backend)
: RHICommandContext()
{
    this->pool = ResourceCast(pool);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = this->pool->GetHandle();
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(backend.GetLogicalDevice(), &allocInfo, &handle) != VK_SUCCESS) 
    {
        LOG_FATAL("Failed to allocate command buffer!");
    }
}

void VulkanRHICommandContext::Destroy() 
{
    vkFreeCommandBuffers(Backend()->GetLogicalDevice(), pool->GetHandle(), 1, &handle);
}

void VulkanRHICommandContext::BeginCommand()
{
    vkResetCommandBuffer(handle, 0);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;

    vkBeginCommandBuffer(handle, &beginInfo);
}

void VulkanRHICommandContext::EndCommand()
{
    if (vkEndCommandBuffer(handle) != VK_SUCCESS) 
    {
        LOG_FATAL("Failed to end command buffer!");
    }
}

void VulkanRHICommandContext::Execute(RHIFenceRef fence, RHISemaphoreRef waitSemaphore, RHISemaphoreRef signalSemaphore)
{
    VkPipelineStageFlags stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;   
    VkFence signalFence = VK_NULL_HANDLE;

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &handle;

    if (fence != nullptr)
    {
        signalFence = ResourceCast(fence)->GetHandle();
    }
    if (waitSemaphore != nullptr)
    {  
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &ResourceCast(waitSemaphore)->GetHandle();
        submitInfo.pWaitDstStageMask = &stage;
    }
    if (signalSemaphore != nullptr)
    {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &ResourceCast(signalSemaphore)->GetHandle();
    }

    if (vkQueueSubmit(ResourceCast(pool->GetQueue())->GetHandle(), 1, &submitInfo, signalFence) != VK_SUCCESS) 
    {
        LOG_FATAL("Failed to submit command buffer!");
    }
}

void VulkanRHICommandContext::TextureBarrier(const RHITextureBarrier& barrier)
{
    ::TextureBarrier(handle, barrier);
}

void VulkanRHICommandContext::BufferBarrier(const RHIBufferBarrier& barrier)
{
    ::BufferBarrier(handle, barrier);
}

void VulkanRHICommandContext::CopyTextureToBuffer(RHITextureRef src, TextureSubresourceLayers srcSubresource, RHIBufferRef dst, uint64_t dstOffset)
{
    ::CopyTextureToBuffer(handle, src, srcSubresource, dst, dstOffset);
}

void VulkanRHICommandContext::CopyBufferToTexture(RHIBufferRef src, uint64_t srcOffset, RHITextureRef dst, TextureSubresourceLayers dstSubresource)
{
    ::CopyBufferToTexture(handle, src, srcOffset, dst, dstSubresource);
}

void VulkanRHICommandContext::CopyBuffer(RHIBufferRef src, uint64_t srcOffset, RHIBufferRef dst, uint64_t dstOffset, uint64_t size)
{
    ::CopyBuffer(handle, src, srcOffset, dst, dstOffset, size);
}

void VulkanRHICommandContext::CopyTexture(RHITextureRef src, TextureSubresourceLayers srcSubresource, RHITextureRef dst, TextureSubresourceLayers dstSubresource)
{
    ::CopyTexture(handle, src, srcSubresource, dst, dstSubresource);
}

void VulkanRHICommandContext::GenerateMips(RHITextureRef src)
{
    ::GenerateMips(handle, src);
} 

void VulkanRHICommandContext::PushEvent(const std::string& name, Color3 color) 
{
    VkDebugUtilsLabelEXT label_info;
    label_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    label_info.pNext = nullptr;
    label_info.pLabelName = name.c_str();
    label_info.color[0] = color.r;
    label_info.color[1] = color.g;
    label_info.color[2] = color.b;
    label_info.color[3] = 1.0f;

    vkCmdBeginDebugUtilsLabelEXT(handle, &label_info);
}   

void VulkanRHICommandContext::PopEvent() 
{
    vkCmdEndDebugUtilsLabelEXT(handle);
}

void VulkanRHICommandContext::BeginRenderPass(RHIRenderPassRef renderPass) 
{
    std::vector<VkClearValue> clearValues;
    for(uint32_t i = 0; i < MAX_RENDER_TARGETS; i++) 
    {
        auto& attachment = ResourceCast(renderPass)->GetInfo().colorAttachments[i];
        if(attachment.textureView == nullptr) break;

        VkClearValue clearValue = {};
        clearValue.color = { {attachment.clearColor.r, attachment.clearColor.g, 
                                    attachment.clearColor.b, attachment.clearColor.a} };
        clearValues.push_back(clearValue);
    }
    const auto& depthAttachment = ResourceCast(renderPass)->GetInfo().depthStencilAttachment;
    if(depthAttachment.textureView != nullptr)
    {
        VkClearValue clearValue = {};
        clearValue.depthStencil = { depthAttachment.clearDepth, depthAttachment.clearStencil };
        clearValues.push_back(clearValue);
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = ResourceCast(renderPass)->GetHandle();
    renderPassInfo.framebuffer = ResourceCast(renderPass)->GetFrameBuffer();
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = VulkanUtil::ExtentToVk(ResourceCast(renderPass)->GetInfo().extent);
    renderPassInfo.clearValueCount = (uint32_t)clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(handle, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    this->renderPass = ResourceCast(renderPass).get();
}  

void VulkanRHICommandContext::EndRenderPass() 
{
    vkCmdEndRenderPass(handle);
    this->renderPass = nullptr;
}

void VulkanRHICommandContext::SetViewport(Offset2D min, Offset2D max) 
{
    VkViewport viewport{};
    viewport.x = (float)min.x;
    viewport.y = (float)min.y;
    viewport.width = (float)(max.x - min.x);
    viewport.height = (float)(max.y - min.y);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(handle, 0, 1, &viewport);
}

void VulkanRHICommandContext::SetScissor(Offset2D min, Offset2D max) 
{
    VkRect2D scissor{};
    scissor.offset = {min.x, min.y};
    scissor.extent = {(uint32_t)(max.x - min.x), (uint32_t)(max.y - min.y)};
    vkCmdSetScissor(handle, 0, 1, &scissor);
}

void VulkanRHICommandContext::SetGraphicsPipeline(RHIGraphicsPipelineRef graphicsPipeline) 
{
    this->graphicsPipeline = ResourceCast(graphicsPipeline).get();
    this->computePipeline = nullptr;
    this->rayTraycingPipeline =  nullptr;

    this->graphicsPipeline->Bind(handle);
}

void VulkanRHICommandContext::SetComputePipeline(RHIComputePipelineRef computePipeline) 
{
    this->graphicsPipeline = nullptr;
    this->computePipeline = ResourceCast(computePipeline).get();
    this->rayTraycingPipeline = nullptr;

    this->computePipeline->Bind(handle);
}	

void VulkanRHICommandContext::PushConstants(void* data, uint16_t size, ShaderFrequency frequency) 
{
    vkCmdPushConstants(handle, 
        GetCuttentPipelineLayout(), 
        VulkanUtil::ShaderFrequencyToVkStageFlags(frequency), 
        0, size, data);
}

void VulkanRHICommandContext::BindDescriptorSet(RHIDescriptorSetRef descriptor, uint32_t set)
{
    vkCmdBindDescriptorSets(handle, 
    GetCuttentBindingPoint(), 
    GetCuttentPipelineLayout(), 
    set, 1, &ResourceCast(descriptor)->GetHandle(), 
    0,              //TODO dynamic offset
    nullptr);
}

void VulkanRHICommandContext::BindVertexBuffer(RHIBufferRef vertexBuffer, uint32_t streamIndex, uint32_t offset)
{
    VkDeviceSize offsets = offset;
    vkCmdBindVertexBuffers(handle, streamIndex, 1, &ResourceCast(vertexBuffer)->GetHandle(), &offsets);
}

void VulkanRHICommandContext::BindIndexBuffer(RHIBufferRef indexBuffer, uint32_t offset)
{
    vkCmdBindIndexBuffer(handle, ResourceCast(indexBuffer)->GetHandle(), offset, VK_INDEX_TYPE_UINT32);    // 固定了索引用32位的
}

void VulkanRHICommandContext::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) 
{
    vkCmdDispatch(handle, groupCountX, groupCountY, groupCountZ);
}

void VulkanRHICommandContext::DispatchIndirect(RHIBufferRef argumentBuffer, uint32_t argumentOffset) 
{

}

void VulkanRHICommandContext::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) 
{
    vkCmdDraw(handle, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanRHICommandContext::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) 
{
    vkCmdDrawIndexed(handle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanRHICommandContext::DrawIndirect(RHIBufferRef argumentBuffer, uint32_t offset, uint32_t drawCount) 
{
    vkCmdDrawIndirect(handle, ResourceCast(argumentBuffer)->GetHandle(), offset, drawCount, sizeof(RHIIndirectCommand));
}

void VulkanRHICommandContext::DrawIndexedIndirect(RHIBufferRef argumentBuffer, uint32_t offset, uint32_t drawCount)
{
    vkCmdDrawIndexedIndirect(handle, ResourceCast(argumentBuffer)->GetHandle(), offset, drawCount, sizeof(RHIIndexedIndirectCommand));
}

VkPipelineLayout VulkanRHICommandContext::GetCuttentPipelineLayout()
{
    if (graphicsPipeline != nullptr)    return graphicsPipeline->GetPipelineLayout();
    if (computePipeline != nullptr)     return computePipeline->GetPipelineLayout();
    // TODO 

    LOG_FATAL("Havent bind any pipeline!"); return nullptr;
}

VkPipelineBindPoint VulkanRHICommandContext::GetCuttentBindingPoint()
{
    if (graphicsPipeline != nullptr)        return VK_PIPELINE_BIND_POINT_GRAPHICS;
    if (computePipeline != nullptr)         return VK_PIPELINE_BIND_POINT_COMPUTE;
    if (rayTraycingPipeline != nullptr)     return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;

    LOG_FATAL("Havent bind any pipeline!"); return VK_PIPELINE_BIND_POINT_MAX_ENUM;
}







VulkanRHIImmediateCommand::VulkanRHIImmediateCommand(VulkanRHIBackend& backend)
{
    fence = backend.CreateFence();
    queue = backend.GetQueue({QUEUE_TYPE_GRAPHICS, 0});
    commandPool = backend.CreateCommandPool({queue});
}

void VulkanRHIImmediateCommand::TextureBarrier(const RHITextureBarrier& barrier)
{
    VkCommandBuffer handle = BeginSingleTimeCommand();
    ::TextureBarrier(handle, barrier);
    EndSingleTimeCommand(handle);
}

void VulkanRHIImmediateCommand::BufferBarrier(const RHIBufferBarrier& barrier)
{
    VkCommandBuffer handle = BeginSingleTimeCommand();
    ::BufferBarrier(handle, barrier);
    EndSingleTimeCommand(handle);
}

void VulkanRHIImmediateCommand::CopyTextureToBuffer(RHITextureRef src, TextureSubresourceLayers srcSubresource, RHIBufferRef dst, uint64_t dstOffset)
{
    VkCommandBuffer handle = BeginSingleTimeCommand();
    ::CopyTextureToBuffer(handle, src, srcSubresource, dst, dstOffset);
    EndSingleTimeCommand(handle);
}

void VulkanRHIImmediateCommand::CopyBufferToTexture(RHIBufferRef src, uint64_t srcOffset, RHITextureRef dst, TextureSubresourceLayers dstSubresource)
{
    VkCommandBuffer handle = BeginSingleTimeCommand();
    ::CopyBufferToTexture(handle, src, srcOffset, dst, dstSubresource);
    EndSingleTimeCommand(handle);
}

void VulkanRHIImmediateCommand::CopyBuffer(RHIBufferRef src, uint64_t srcOffset, RHIBufferRef dst, uint64_t dstOffset, uint64_t size)
{
    VkCommandBuffer handle = BeginSingleTimeCommand();
    ::CopyBuffer(handle, src, srcOffset, dst, dstOffset, size);
    EndSingleTimeCommand(handle);
}

void VulkanRHIImmediateCommand::CopyTexture(RHITextureRef src, TextureSubresourceLayers srcSubresource, RHITextureRef dst, TextureSubresourceLayers dstSubresource)
{
    VkCommandBuffer handle = BeginSingleTimeCommand();
    ::CopyTexture(handle, src, srcSubresource, dst, dstSubresource);
    EndSingleTimeCommand(handle);
}

void VulkanRHIImmediateCommand::GenerateMips(RHITextureRef src)
{
    VkCommandBuffer handle = BeginSingleTimeCommand();
    ::GenerateMips(handle, src);
    EndSingleTimeCommand(handle);
}

VkCommandBuffer VulkanRHIImmediateCommand::BeginSingleTimeCommand()
{
    //新开一个命令缓冲区
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = ResourceCast(commandPool)->GetHandle();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(Backend()->GetLogicalDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanRHIImmediateCommand::EndSingleTimeCommand(VkCommandBuffer commandBuffer)
{
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        LOG_FATAL("Failed to record command buffer!");
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    VkResult result = vkQueueSubmit(ResourceCast(queue)->GetHandle(), 1, &submitInfo, ResourceCast(fence)->GetHandle());
    if (result != VK_SUCCESS) {
        LOG_FATAL("Failed to submit draw command buffer! [%d]", result);
    }

    fence->Wait();
    vkFreeCommandBuffers(Backend()->GetLogicalDevice(), ResourceCast(commandPool)->GetHandle(), 1, &commandBuffer);
}