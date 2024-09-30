// #pragma once

// #include "Function/Framework/Component/TransformComponent.h"
// #include "Function/Framework/Scene/Scene.h"
// #include "Function/Global/EngineContext.h"
// #include "Function/Render/RenderResource/Buffer.h"
// #include "Function/Render/RenderResource/Model.h"
// #include "Function/Render/RenderResource/Texture.h"
// #include "TestMath.h"

// #include "GLFW/glfw3.h"

// extern Extent2D windowsExtent;
// extern Offset2D windowsOffset;
// extern RHIFormat colorFormat;
// extern RHIFormat depthFormat;
// extern uint32_t framesInFlight;
// extern std::string shaderPath;

// /*
// typedef struct Vertex
// {
//     float pos_color[6]; 
// } Vertex;
// const Vertex vertices[] = {
//     {-0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f},
//     {0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f},
//     {0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f},

//     {-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f},
//     {0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f},
//     {0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f},
// };
// const uint32_t indices[] = {
//     0, 1, 2,
//     3, 4, 5,
// };
// */

// static void TestRHI(GLFWwindow* window)
// {
//     std::shared_ptr<Scene> scene = EngineContext::World()->LoadScene("resource/build_in/config/scene/default.scene");
//     std::shared_ptr<Entity> e1 = scene->GetEntity("Entity1");
//     std::shared_ptr<Entity> e2 = scene->GetEntity("Entity2");

//     std::shared_ptr<TransformComponent> transformComponent = e1->TryGetComponent<TransformComponent>();
    
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
//     EngineContext::File()->LoadBinary(shaderPath + "test/forward.frag.spv", fragShaderCode);
//     EngineContext::File()->LoadBinary(shaderPath + "test/deferred_lighting.comp.spv", compShaderCode);

//     // RHI //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//     RHIBackendRef backend = EngineContext::RHI();

//     // surface
//     RHISurfaceRef surface = backend->CreateSurface(window);

//     // queue
//     RHIQueueInfo queueInfo = { 
//         .type = QUEUE_TYPE_GRAPHICS, 
//         .index = 0 };
//     RHIQueueRef queue = backend->GetQueue(queueInfo);

//     // commandPool
//     RHICommandPoolInfo poolInfo = {
//         .queue = queue
//     };
//     RHICommandPoolRef pool = backend->CreateCommandPool(poolInfo);
//     std::vector<RHICommandListRef> commands;
//     for(uint32_t i = 0; i < framesInFlight; i++) commands.push_back(pool->CreateCommandList(false));

//     // swapchain
//     RHISwapchainInfo swapchainInfo = {
//         .surface = surface,
//         .presentQueue = queue,
//         .imageCount = framesInFlight,
//         .extent = surface->GetExetent(),
//         .format = colorFormat };
//     RHISwapchainRef swapchain = backend->CreateSwapChain(swapchainInfo);

//     // buffer
//     // RHIBufferInfo rwBufferInfo = {
//     //     .size = 100 * sizeof(float) * 6,
//     //     .memoryUsage = MEMORY_USAGE_CPU_TO_GPU,
//     //     .type = RESOURCE_TYPE_RW_BUFFER,
//     //     .creationFlag = BUFFER_CREATION_PERSISTENT_MAP
//     // };
//     // RHIBufferRef rwBuffer = backend->CreateBuffer(rwBufferInfo);


//     // RHIBufferInfo vertexBufferInfo = {
//     //     .size = 3 * sizeof(Vertex),
//     //     .memoryUsage = MEMORY_USAGE_CPU_TO_GPU,
//     //     .type = RESOURCE_TYPE_RW_BUFFER | RESOURCE_TYPE_VERTEX_BUFFER,
//     //     .creationFlag = BUFFER_CREATION_PERSISTENT_MAP
//     // };
//     // RHIBufferRef vertexBuffer = backend->CreateBuffer(vertexBufferInfo);
//     // memcpy(vertexBuffer->Map(), &vertices[0], 3 * sizeof(Vertex));

//     // RHIBufferInfo indexBufferInfo = {
//     //     .size = 3 * sizeof(uint32_t), 
//     //     .memoryUsage = MEMORY_USAGE_CPU_TO_GPU,
//     //     .type = RESOURCE_TYPE_RW_BUFFER | RESOURCE_TYPE_INDEX_BUFFER,
//     //     .creationFlag = BUFFER_CREATION_PERSISTENT_MAP
//     // };
//     // RHIBufferRef indexBuffer = backend->CreateBuffer(indexBufferInfo);
//     // memcpy(indexBuffer->Map(), &indices[0], 3 * sizeof(uint32_t));

//     // texture
//     RHITextureInfo colorTextureInfo = {
//         .format = colorFormat,
//         .extent = { windowsExtent.width, windowsExtent.height, 1},
//         .arrayLayers = 1,
//         .mipLevels = 1,
//         .memoryUsage = MEMORY_USAGE_GPU_ONLY,
//         .type = RESOURCE_TYPE_TEXTURE | RESOURCE_TYPE_RW_TEXTURE | RESOURCE_TYPE_RENDER_TARGET,
//         .creationFlag = TEXTURE_CREATION_NONE
//     };
//     RHITextureRef colorTexture = backend->CreateTexture(colorTextureInfo);


//     RHITextureInfo depthTextureInfo = {
//         .format = depthFormat,
//         .extent = { windowsExtent.width, windowsExtent.height, 1},
//         .arrayLayers = 1,
//         .mipLevels = 1,
//         .memoryUsage = MEMORY_USAGE_GPU_ONLY,
//         .type = RESOURCE_TYPE_TEXTURE | RESOURCE_TYPE_RENDER_TARGET,
//         .creationFlag = TEXTURE_CREATION_NONE
//     };
//     RHITextureRef depthTexture = backend->CreateTexture(depthTextureInfo);

//     // texture view
//     RHITextureViewInfo colorTextureViewInfo = {
//         .texture = colorTexture,
//         .format = colorTexture->GetInfo().format,
//         .viewType = VIEW_TYPE_2D,
//         .subresource = {TEXTURE_ASPECT_COLOR, 0, 1, 0, 1},
//     };
//     RHITextureViewRef colorTextureView = backend->CreateTextureView(colorTextureViewInfo);


//     RHITextureViewInfo depthTextureViewInfo = {
//         .texture = depthTexture,
//         .format = depthTexture->GetInfo().format,
//         .viewType = VIEW_TYPE_2D,
//         .subresource = {TEXTURE_ASPECT_DEPTH_STENCIL, 0, 1, 0, 1},
//         };
//     RHITextureViewRef depthTextureView = backend->CreateTextureView(depthTextureViewInfo);

//     // sampler
//     RHISamplerInfo samplerInfo = {
//         .minFilter = FILTER_TYPE_LINEAR,
//         .magFilter = FILTER_TYPE_LINEAR,
//         .mipmapMode = MIPMAP_MODE_LINEAR,
//         .addressModeU = ADDRESS_MODE_CLAMP_TO_EDGE,
//         .addressModeV = ADDRESS_MODE_CLAMP_TO_EDGE,
//         .addressModeW = ADDRESS_MODE_CLAMP_TO_EDGE,
//         .compareFunction = COMPARE_FUNCTION_NEVER,
//         .mipLodBias = 0.0f,
//         .maxAnisotropy = 0.0f, 
//     };
//     RHISamplerRef sampler = backend->CreateSampler(samplerInfo);

//     // shader
//     RHIShaderInfo vertShaderInfo = {
//         .entry = "main",
//         .frequency = SHADER_FREQUENCY_VERTEX,
//         .code = vertShaderCode
//     };
//     RHIShaderInfo fragShaderInfo = {
//         .entry = "main",
//         .frequency = SHADER_FREQUENCY_FRAGMENT,
//         .code = fragShaderCode
//     };
//     RHIShaderInfo computeShaderInfo = {
//         .entry = "main",
//         .frequency = SHADER_FREQUENCY_COMPUTE,
//         .code = compShaderCode
//     };
//     RHIShaderRef vertShader = backend->CreateShader(vertShaderInfo);
//     RHIShaderRef fragShader = backend->CreateShader(fragShaderInfo);
//     RHIShaderRef compShader = backend->CreateShader(computeShaderInfo);

//     // rootSignature
//     RHIRootSignatureInfo rootSignatureInfo = {};
//     rootSignatureInfo.AddEntry(EngineContext::RenderResource()->GetPerFrameRootSignature()->GetInfo())
//                      //.AddEntryFromReflect(vertShader)
//                      //.AddEntryFromReflect(fragShader)
//                      .AddPushConstant({.size = 128, .frequency = SHADER_FREQUENCY_GRAPHICS})
//                      .AddEntry({1, 0, 1, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_TEXTURE})
//                      .AddEntry({1, 1, 1, SHADER_FREQUENCY_ALL, RESOURCE_TYPE_RW_BUFFER});
//     RHIRootSignatureRef rootSignature = backend->CreateRootSignature(rootSignatureInfo);

//     RHIRootSignatureInfo computeRootSignatureInfo = {};
//     computeRootSignatureInfo.AddEntryFromReflect(compShader);
//     RHIRootSignatureRef computeRootSignature = backend->CreateRootSignature(computeRootSignatureInfo);

//     // descriptorSet
//     // RHIDescriptorSetRef descriptor = rootSignature->CreateDescriptorSet(0);
//     // RHIDescriptorUpdateInfo descriptorUpdateInfo = {
//     //     .binding = 0,
//     //     .index = 0,
//     //     .resourceType = RESOURCE_TYPE_RW_BUFFER,
//     //     .buffer = rwBuffer,
//     //     .bufferOffset = 0,
//     //     .bufferRange = rwBuffer->GetInfo().size
//     // };
//     // descriptor0->UpdateDescriptor(descriptorUpdateInfo);

//     // graphicsPipeline
//     RHIGraphicsPipelineInfo pipelineInfo = {};

//     pipelineInfo.vertexShader = vertShader;
//     pipelineInfo.fragmentShader = fragShader;

//     pipelineInfo.rootSignature = rootSignature;

//     pipelineInfo.primitiveType = PRIMITIVE_TYPE_TRIANGLE_LIST;
    
//     // pipelineInfo.vertexInputState.vertexElements.push_back({
//     //     .streamIndex = 0,
//     //     .attributeIndex = 0, 
//     //     .format = FORMAT_R32G32B32_SFLOAT,
//     //     .offset = 0,
//     //     .stride = 6 * sizeof(float),
//     //     .useInstanceIndex = false,
//     // });
//     // pipelineInfo.vertexInputState.vertexElements.push_back({
//     //     .streamIndex = 0,
//     //     .attributeIndex = 1, 
//     //     .format = FORMAT_R32G32B32_SFLOAT,
//     //     .offset = 3 * sizeof(float),
//     //     .stride = 6 * sizeof(float),
//     //     .useInstanceIndex = false,
//     // });

//     pipelineInfo.rasterizerState = {
//         .fillMode = FILL_MODE_SOLID,
//         .cullMode = CULL_MODE_NONE,
//         .depthClipMode = DEPTH_CLIP,
//         .depthBias = 0.0f,
//         .slopeScaleDepthBias = 0.0f
//     };

//     pipelineInfo.blendState.renderTargets[0] = {
//         .enable = false,
//         .colorBlendOp = BLEND_OP_ADD,
// 		.colorSrcBlend = BLEND_FACTOR_ONE,
// 		.colorDstBlend = BLEND_FACTOR_ZERO,
// 		.alphaBlendOp = BLEND_OP_ADD,
// 		.alphaSrcBlend = BLEND_FACTOR_ONE,
// 		.alphaDstBlend = BLEND_FACTOR_ZERO,
// 		.colorWriteMask = COLOR_MASK_RGBA
//     };

//     pipelineInfo.depthStencilState = {
//         .enableDepthTest = true,
//         .enableDepthWrite = true,
//         .depthTest = COMPARE_FUNCTION_LESS_EQUAL
//     };

//     pipelineInfo.colorAttachmentFormats[0] = colorFormat;
//     pipelineInfo.depthStencilAttachmentFormat = depthFormat;

//     RHIGraphicsPipelineRef graphicsPipeline = backend->CreateGraphicsPipeline(pipelineInfo);

//     // computePipeline
//     RHIComputePipelineInfo computePipelineInfo = {
//         .computeShader = compShader,
//         .rootSignature = computeRootSignature
//     };
//     RHIComputePipelineRef computePipeline = backend->CreateComputePipeline(computePipelineInfo);
    
//     // renderPass
//     RHIRenderPassInfo renderPassInfo = {
//         .extent = windowsExtent,
//         .layers = 1
//     };
//     renderPassInfo.colorAttachments[0] = {
//         .textureView = colorTextureView,
//         .loadOp = ATTACHMENT_LOAD_OP_CLEAR,
//         .storeOp = ATTACHMENT_STORE_OP_STORE,
//         .clearColor = {0.0f, 0.0f, 0.0f, 0.0f}
//     };
//     renderPassInfo.depthStencilAttachment = {
//         .textureView = depthTextureView,
//         .loadOp = ATTACHMENT_LOAD_OP_CLEAR,
//         .storeOp = ATTACHMENT_STORE_OP_STORE,
//         .clearDepth = 1.0f
//     };
//     RHIRenderPassRef renderPass = backend->CreateRenderPass(renderPassInfo);


//     // fence semaphore
//     std::vector<RHISemaphoreRef> startSemaphores;
//     std::vector<RHISemaphoreRef> finishSemaphores;
//     std::vector<RHIFenceRef> fences;
//     for(uint32_t i = 0; i < framesInFlight; i++)
//     {
//         startSemaphores.push_back(backend->CreateSemaphore());
//         finishSemaphores.push_back(backend->CreateSemaphore());
//         fences.push_back(backend->CreateFence(true));
//     }

//     bool initResource = false;
//     while(!glfwWindowShouldClose(window)) 
//     {
//         glfwPollEvents();

//         EngineContext::Tick(); 
//         uint32_t frameIndex = EngineContext::CurrentFrameIndex();

//         Move(transformComponent, EngineContext::GetDeltaTime(), window);    // 镜头移动



//         fences[frameIndex]->Wait();
//         RHITextureRef swapchainTexture = swapchain->GetNewFrame(nullptr, startSemaphores[frameIndex]);      

//         RHICommandListRef command = commands[frameIndex];
//         command->BeginCommand();
        
//         if(!initResource)
//         {
//             initResource = true;
//             // command->BufferBarrier({indexBuffer, RESOURCE_STATE_UNDEFINED, RESOURCE_STATE_INDEX_BUFFER});
//             // command->BufferBarrier({vertexBuffer, RESOURCE_STATE_UNDEFINED, RESOURCE_STATE_VERTEX_BUFFER});
//             // command->BufferBarrier({rwBuffer, RESOURCE_STATE_UNDEFINED, RESOURCE_STATE_UNORDERED_ACCESS});
//             command->TextureBarrier({colorTexture, RESOURCE_STATE_UNDEFINED, RESOURCE_STATE_COLOR_ATTACHMENT});
//             command->TextureBarrier({depthTexture, RESOURCE_STATE_UNDEFINED, RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT});
//         }     

//         {
//             command->PushEvent("RHI Render Pass", {0.0f, 0.0f, 0.0f});

//             command->BeginRenderPass(renderPass);
//             command->SetViewport({0, 0}, windowsOffset);
//             command->SetScissor({0, 0}, windowsOffset); 
//             command->SetGraphicsPipeline(graphicsPipeline);
//             command->BindDescriptorSet(EngineContext::RenderResource()->GetPerFrameDescriptorSet(), 0);
//             // command->BindDescriptorSet(descriptor1, 1);
//             // command->PushConstants(&pushConstants, 128, SHADER_FREQUENCY_GRAPHICS);
//             for(uint32_t i = 0; i < model->GetSubmeshCount(); i++)
//             {
//                 VertexBufferRef vertexBuffer = model->GetVertexBuffer(i);
//                 IndexBufferRef indexBuffer = model->GetIndexBuffer(i);
//                 // command->BindVertexBuffer(vertexBuffer->buffer, 0, 0);
//                 // command->BindIndexBuffer(indexBuffer->buffer, 0);
//                 // command->DrawIndexed(indexBuffer->IndexNum(), 1, 0, 0, i + 1); 

//                 command->Draw(indexBuffer->IndexNum(), 1, 0, i + 1);
//             }  
//             command->EndRenderPass();

//             command->TextureBarrier({colorTexture, RESOURCE_STATE_COLOR_ATTACHMENT, RESOURCE_STATE_TRANSFER_SRC});
//             command->TextureBarrier({swapchainTexture, RESOURCE_STATE_PRESENT, RESOURCE_STATE_TRANSFER_DST});
//             command->CopyTexture(   colorTexture, {TEXTURE_ASPECT_COLOR, 0, 0, 1}, 
//                                     swapchainTexture, {TEXTURE_ASPECT_COLOR, 0, 0, 1});
//             command->TextureBarrier({colorTexture, RESOURCE_STATE_TRANSFER_SRC, RESOURCE_STATE_COLOR_ATTACHMENT});
//             command->TextureBarrier({swapchainTexture, RESOURCE_STATE_TRANSFER_DST, RESOURCE_STATE_PRESENT});

//             command->PopEvent();
//         }


//         command->EndCommand();
//         command->Execute(fences[frameIndex], startSemaphores[frameIndex], finishSemaphores[frameIndex]);

//         swapchain->Present(finishSemaphores[frameIndex]);       
//     }


// }