// #pragma once

// #include "Function/Framework/Component/CameraComponent.h"
// #include "Function/Framework/Component/TransformComponent.h"
// #include "Function/Framework/Scene/Scene.h"
// #include "Function/Global/EngineContext.h"
// #include "Function/Render/RDG/RDGBuilder.h"
// #include "Function/Render/RHI/RHIStructs.h"
// #include "Function/Render/RenderResource/Buffer.h"
// #include "Function/Render/RenderResource/Model.h"
// #include "Function/Render/RenderResource/Texture.h"
// #include "TestMath.h"

// #include "GLFW/glfw3.h"
// #include <cstdint>
// #include <cstring>
// #include <memory>
// #include <vector>

// extern Extent2D windowsExtent;
// extern Offset2D windowsOffset;
// extern RHIFormat colorFormat;
// extern RHIFormat depthFormat;
// extern uint32_t framesInFlight;
// extern std::string shaderPath;

// static void Mode(RHIBufferRef settingBuffer, GLFWwindow* window)
// {
//     static uint32_t mode = 0;
//     static RHIBufferRef buffer;

//     if(buffer == nullptr)
//     {
//         buffer = settingBuffer;
//         glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods){
//             if (key == GLFW_KEY_Q && action == GLFW_PRESS) { mode--; memcpy(buffer->Map(), &mode, sizeof(mode)); } 
//             if (key == GLFW_KEY_E && action == GLFW_PRESS) { mode++; memcpy(buffer->Map(), &mode, sizeof(mode)); }  
//         });
//     }             
// }

// static void TestRDG(GLFWwindow* window)
// {
//     std::shared_ptr<Scene> scene    = EngineContext::World()->LoadScene("resource/build_in/config/scene/default.scene");
//     std::shared_ptr<Entity> e1      = scene->GetEntity("Entity1");
//     std::shared_ptr<Entity> e2      = scene->GetEntity("Entity2");

//     std::shared_ptr<TransformComponent> transformComponent = e1->TryGetComponent<TransformComponent>();
//     std::shared_ptr<CameraComponent> cameraComponent = e1->TryGetComponent<CameraComponent>();
    
//     EngineContext::World()->SetActiveScene("defaultScene");

//     std::shared_ptr<Model> model    = EngineContext::Asset()->GetOrLoadAsset<Model>("resource/build_in/config/model/klee.model");
//     std::vector<TextureRef> textures;
//     textures.push_back(std::make_shared<Texture>("resource/build_in/model/Klee/Texture/face.jpg")); 
//     textures.push_back(std::make_shared<Texture>("resource/build_in/model/Klee/Texture/hair.jpg")); 
//     textures.push_back(EngineContext::Asset()->GetOrLoadAsset<Texture>("resource/build_in/config/texture/dressing.texr"));    


//     std::vector<uint8_t> vertShaderCode;
//     std::vector<uint8_t> fragShaderCode;
//     std::vector<uint8_t> compShaderCode;
//     EngineContext::File()->LoadBinary(shaderPath + "test/default.vert.spv", vertShaderCode);
//     EngineContext::File()->LoadBinary(shaderPath + "test/deferred.frag.spv", fragShaderCode);
//     EngineContext::File()->LoadBinary(shaderPath + "test/deferred_lighting.comp.spv", compShaderCode);

//     // RDG //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
//     RHIBackendRef backend       = EngineContext::RHI();
//     RHISurfaceRef surface       = backend->CreateSurface(window);
//     RHIQueueRef queue           = backend->GetQueue({ QUEUE_TYPE_GRAPHICS, 0 });
//     RHISwapchainRef swapchain   = backend->CreateSwapChain({ surface, queue, framesInFlight, surface->GetExetent(), colorFormat });
//     RHICommandPoolRef pool      = backend->CreateCommandPool({ queue });
//     std::vector<RHICommandListRef> commands;
//     for(uint32_t i = 0; i < framesInFlight; i++) commands.push_back(pool->CreateCommandList(false));





//     RHIShaderRef vertShader = backend->CreateShader({ "main", SHADER_FREQUENCY_VERTEX, vertShaderCode });
//     RHIShaderRef fragShader = backend->CreateShader({ "main", SHADER_FREQUENCY_FRAGMENT, fragShaderCode });
//     RHIShaderRef compShader = backend->CreateShader({ "main", SHADER_FREQUENCY_COMPUTE, compShaderCode });
//     RHIBufferRef settingBuffer = backend->CreateBuffer({128, MEMORY_USAGE_CPU_TO_GPU, RESOURCE_TYPE_UNIFORM_BUFFER, BUFFER_CREATION_PERSISTENT_MAP});
//     RHIBufferRef indirectBuffer = backend->CreateBuffer({sizeof(RHIIndirectCommand) * 128, MEMORY_USAGE_CPU_TO_GPU, RESOURCE_TYPE_RW_BUFFER | RESOURCE_TYPE_INDIRECT_BUFFER, BUFFER_CREATION_PERSISTENT_MAP});

//     RHIRootSignatureInfo graphicsRootSignatureInfo = {};
//     graphicsRootSignatureInfo.AddEntry(EngineContext::RenderResource()->GetPerFrameRootSignature()->GetInfo());                      
//     RHIRootSignatureRef graphicsRootSignature = backend->CreateRootSignature(graphicsRootSignatureInfo);

//     RHIRootSignatureInfo computeRootSignatureInfo = {};
//     computeRootSignatureInfo.AddEntry({0, 0, 1, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_TEXTURE})
//                             .AddEntry({0, 1, 1, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_TEXTURE})
//                             .AddEntry({0, 2, 1, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_TEXTURE})
//                             .AddEntry({0, 3, 1, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_TEXTURE})
//                             .AddEntry({0, 4, 1, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_TEXTURE})
//                             .AddEntry({0, 5, 1, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_UNIFORM_BUFFER});
//     RHIRootSignatureRef computeRootSignature = backend->CreateRootSignature(computeRootSignatureInfo);

//     RHIGraphicsPipelineInfo pipelineInfo = {};
//     pipelineInfo.vertexShader       = vertShader;
//     pipelineInfo.fragmentShader     = fragShader;
//     pipelineInfo.rootSignature      = graphicsRootSignature;
//     pipelineInfo.primitiveType      = PRIMITIVE_TYPE_TRIANGLE_LIST;
//     pipelineInfo.rasterizerState    = { FILL_MODE_SOLID, CULL_MODE_BACK, DEPTH_CLIP, 0.0f, 0.0f };
//     for(uint32_t i = 0; i < 4; i++) pipelineInfo.blendState.renderTargets[i].enable = false;
//     pipelineInfo.colorAttachmentFormats[0]      = FORMAT_R8G8B8A8_UNORM;
//     pipelineInfo.colorAttachmentFormats[1]      = FORMAT_R8G8B8A8_UNORM;
//     pipelineInfo.colorAttachmentFormats[2]      = FORMAT_R8G8B8A8_UNORM;
//     pipelineInfo.colorAttachmentFormats[3]      = FORMAT_R16G16B16A16_SFLOAT;                                               
//     pipelineInfo.depthStencilState              = { true, true, COMPARE_FUNCTION_LESS_EQUAL };
//     pipelineInfo.depthStencilAttachmentFormat   = depthFormat;
//     RHIGraphicsPipelineRef graphicsPipeline     = backend->CreateGraphicsPipeline(pipelineInfo);


//     RHIComputePipelineInfo computePipelineInfo  = {};
//     computePipelineInfo.computeShader           = compShader;
//     computePipelineInfo.rootSignature           = computeRootSignature;
//     RHIComputePipelineRef computePipeline       = backend->CreateComputePipeline(computePipelineInfo);


    

//     std::vector<RHISemaphoreRef> startSemaphores;
//     std::vector<RHISemaphoreRef> finishSemaphores;
//     std::vector<RHIFenceRef> fences;
//     for(uint32_t i = 0; i < framesInFlight; i++)
//     {
//         startSemaphores.push_back(backend->CreateSemaphore());
//         finishSemaphores.push_back(backend->CreateSemaphore());
//         fences.push_back(backend->CreateFence(true));
//     }

//     while(!glfwWindowShouldClose(window)) 
//     {
//         glfwPollEvents();

//         EngineContext::Tick(); 
//         uint32_t frameIndex = EngineContext::CurrentFrameIndex();

//         Move(transformComponent, EngineContext::GetDeltaTime(), window);    // 镜头移动   
//         Mode(settingBuffer, window);                                                  // 显示模式
        
//         fences[frameIndex]->Wait();
//         RHITextureRef swapchainTexture = swapchain->GetNewFrame(nullptr, startSemaphores[frameIndex]); 

//         RHICommandListRef command = commands[frameIndex];
//         command->BeginCommand();

//         {
//             RDGBuilder rdgBuilder = RDGBuilder(command);

//             RDGTextureHandle finalColor = rdgBuilder.CreateTexture("Final Color Texture")
//                 .Exetent({windowsExtent.width, windowsExtent.height, 1})
//                 .Format(colorFormat)
//                 .ArrayLayers(1)
//                 .MipLevels(1)
//                 .MemoryUsage(MEMORY_USAGE_GPU_ONLY)
//                 .AllowReadWrite()
//                 .Finish();   
            
//             RDGTextureHandle depth = rdgBuilder.CreateTexture("Depth Texture")
//                 .Exetent({windowsExtent.width, windowsExtent.height, 1})
//                 .Format(depthFormat)
//                 .ArrayLayers(1)
//                 .MipLevels(1)
//                 .MemoryUsage(MEMORY_USAGE_GPU_ONLY)
//                 .AllowDepthStencil()
//                 .Finish();  

//             RDGTextureHandle diffuse = rdgBuilder.CreateTexture("G-Buffer Diffuse Texture")
//                 .Exetent({windowsExtent.width, windowsExtent.height, 1})
//                 .Format(FORMAT_R8G8B8A8_UNORM)
//                 .ArrayLayers(1)
//                 .MipLevels(1)
//                 .MemoryUsage(MEMORY_USAGE_GPU_ONLY)
//                 .AllowReadWrite()
//                 .AllowRenderTarget()
//                 .Finish();

//             RDGTextureHandle normal = rdgBuilder.CreateTexture("G-Buffer Normal Texture")
//                 .Exetent({windowsExtent.width, windowsExtent.height, 1})
//                 .Format(FORMAT_R8G8B8A8_UNORM)
//                 .ArrayLayers(1)
//                 .MipLevels(1)
//                 .MemoryUsage(MEMORY_USAGE_GPU_ONLY)
//                 .AllowReadWrite()
//                 .AllowRenderTarget()
//                 .Finish();

//             RDGTextureHandle arm = rdgBuilder.CreateTexture("G-Buffer AO/Roughness/Metallic Texture")
//                 .Exetent({windowsExtent.width, windowsExtent.height, 1})
//                 .Format(FORMAT_R8G8B8A8_UNORM)
//                 .ArrayLayers(1)
//                 .MipLevels(1)
//                 .MemoryUsage(MEMORY_USAGE_GPU_ONLY)
//                 .AllowReadWrite()
//                 .AllowRenderTarget()
//                 .Finish();
          
//             RDGTextureHandle pos = rdgBuilder.CreateTexture("G-Buffer Position Texture")
//                 .Exetent({windowsExtent.width, windowsExtent.height, 1})
//                 .Format(FORMAT_R16G16B16A16_SFLOAT)
//                 .ArrayLayers(1)
//                 .MipLevels(1)
//                 .MemoryUsage(MEMORY_USAGE_GPU_ONLY)
//                 .AllowReadWrite()
//                 .AllowRenderTarget()
//                 .Finish();

//             RDGTextureHandle present = rdgBuilder.CreateTexture("Present Texture")
//                 .Import(swapchainTexture, RESOURCE_STATE_PRESENT)
//                 .Finish();

//             RDGBufferHandle setting = rdgBuilder.CreateBuffer("Setting Buffer")
//                 // .Size(128)
//                 // .MemoryUsage(MEMORY_USAGE_CPU_TO_GPU)
//                 // .AllowRead()
//                 .Import(settingBuffer, RESOURCE_STATE_SHADER_RESOURCE)
//                 .Finish();

//             RDGRenderPassHandle rdgRenderPass = rdgBuilder.CreateRenderPass("G-Buffer Pass")
//                 .RootSignature(graphicsRootSignature)
//                 .Color(0, diffuse, ATTACHMENT_LOAD_OP_CLEAR, ATTACHMENT_STORE_OP_STORE, {0.0f, 0.0f, 0.0f, 0.0f})
//                 .Color(1, normal, ATTACHMENT_LOAD_OP_CLEAR, ATTACHMENT_STORE_OP_STORE, {0.0f, 0.0f, 0.0f, 0.0f})
//                 .Color(2, arm, ATTACHMENT_LOAD_OP_CLEAR, ATTACHMENT_STORE_OP_STORE, {0.0f, 0.0f, 0.0f, 0.0f})
//                 .Color(3, pos, ATTACHMENT_LOAD_OP_CLEAR, ATTACHMENT_STORE_OP_STORE, {0.0f, 0.0f, 0.0f, 0.0f})
//                 .DepthStencil(depth, ATTACHMENT_LOAD_OP_CLEAR, ATTACHMENT_STORE_OP_STORE, 1.0f, 0)
//                 .Execute([&](RDGPassContext context) {

//                     RHICommandListRef command = context.command;                                            
//                     command->SetViewport({0, 0}, windowsOffset);
//                     command->SetScissor({0, 0}, windowsOffset); 
//                     command->SetGraphicsPipeline(graphicsPipeline);
//                     command->BindDescriptorSet(EngineContext::RenderResource()->GetPerFrameDescriptorSet(), 0);   

//                     // 直接绘制
//                     if(false)
//                     {
//                         for(uint32_t i = 0; i < model->GetSubmeshCount(); i++)
//                         {
//                             VertexBufferRef vertexBuffer = model->GetVertexBuffer(i);
//                             IndexBufferRef indexBuffer = model->GetIndexBuffer(i);
//                             // command->BindVertexBuffer(vertexBuffer->buffer, 0, 0);     // 也可以用绑定的方式绘制
//                             // command->BindIndexBuffer(indexBuffer->buffer, 0);
//                             // command->DrawIndexed(indexBuffer->IndexNum(), 1, 0, 0, i + 1); 

//                             command->Draw(indexBuffer->IndexNum(), 1, 0, i + 1);    
//                         } 
//                     }   

//                     // 间接绘制
//                     if(true)
//                     {
//                         std::vector<RHIIndirectCommand> indirectCommands;
//                         for(uint32_t i = 0; i < model->GetSubmeshCount(); i++)
//                         {
//                             IndexBufferRef indexBuffer = model->GetIndexBuffer(i);

//                             indirectCommands.push_back({
//                                 .vertexCount = indexBuffer->IndexNum(),
//                                 .instanceCount = 1,
//                                 .firstVertex = 0,
//                                 .firstInstance = i + 1, // 对应于Bindless的索引下标(0无效)
//                             }); 
//                         } 
//                         memcpy(indirectBuffer->Map(), indirectCommands.data(), indirectCommands.size() * sizeof(RHIIndirectCommand));
//                         command->DrawIndirect(indirectBuffer, 0, indirectCommands.size());
//                     }
                                 
//                 })
//                 .Finish();

//             RDGComputePassHandle rdgComputePass = rdgBuilder.CreateComputePass("Deferred Lighting Pass")
//                 .RootSignature(computeRootSignature)
//                 // .DescriptorSet(uint32_t set, RHIDescriptorSetRef descriptorSet)
//                 .ReadWrite(0, 0, 0, diffuse)
//                 .ReadWrite(0, 1, 0, normal)
//                 .ReadWrite(0, 2, 0, arm)
//                 .ReadWrite(0, 3, 0, pos)
//                 .ReadWrite(0, 4, 0, finalColor)
//                 .Read(0, 5, 0, setting)
//                 .Execute([&](RDGPassContext context) {       

//                     RHICommandListRef command = context.command; 
//                     command->SetComputePipeline(computePipeline);
//                     command->BindDescriptorSet(context.descriptors[0], 0);
//                     command->Dispatch(  windowsExtent.width / 16, 
//                                         windowsExtent.height / 16, 
//                                         1);
//                 })
//                 .Finish();

//             RDGPresentPassHandle rdgPresentPass = rdgBuilder.CreatePresentPass("Present Pass")
//                 .PresentTexture(present)
//                 .Texture(finalColor)
//                 .Finish();

//             rdgBuilder.Execute();

//             // ENGINE_LOG_INFO("{} {} {} {} {} {}", 
//             //     RDGBufferPool::Get()->PooledSize(), RDGTexturePool::Get()->PooledSize(), RDGTextureViewPool::Get()->PooledSize(),
//             //     RDGBufferPool::Get()->AllocatedSize(), RDGTexturePool::Get()->AllocatedSize(), RDGTextureViewPool::Get()->AllocatedSize());
//         }
         
//         command->EndCommand();
//         command->Execute(fences[frameIndex], startSemaphores[frameIndex], finishSemaphores[frameIndex]);

//         swapchain->Present(finishSemaphores[frameIndex]);   
//     }
    
// }