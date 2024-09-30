#include "GBufferPass.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RDG/RDGBuilder.h"
#include "Function/Render/RHI/RHIStructs.h"
#include "Function/Render/RenderResource/Material.h"
#include "MeshPass.h"
#include "RenderPass.h"
#include <memory>

void GBufferPassProcessor::OnCollectBatch(const DrawBatch& batch)
{
    if(batch.material->RenderPassMask() & PASS_MASK_DEFERRED_PASS) AddBatch(batch);
}

RHIGraphicsPipelineRef GBufferPassProcessor::OnCreatePipeline(const DrawPipelineState& pipelineState)
{
    RHIGraphicsPipelineRef pipeline;

    RHIGraphicsPipelineInfo pipelineInfo    = {};
    pipelineInfo.vertexShader               = pipelineState.vertexShader;
    pipelineInfo.geometryShader             = pipelineState.geometryShader;
    pipelineInfo.fragmentShader             = pipelineState.fragmentShader;
    pipelineInfo.primitiveType              = PRIMITIVE_TYPE_TRIANGLE_LIST;
    pipelineInfo.rasterizerState            = { pipelineState.fillMode, pipelineState.cullMode, DEPTH_CLIP, 0.0f, 0.0f };
    pipelineInfo.depthStencilState          = { pipelineState.depthCompare, pipelineState.depthTest, pipelineState.depthWrite };

    pipelineInfo.rootSignature                  = pass->rootSignature;
    for(uint32_t i = 0; i < 4; i++) pipelineInfo.blendState.renderTargets[i].enable = false;
    pipelineInfo.colorAttachmentFormats[0]      = FORMAT_R8G8B8A8_UNORM;
    pipelineInfo.colorAttachmentFormats[1]      = FORMAT_R8G8B8A8_UNORM;
    pipelineInfo.colorAttachmentFormats[2]      = FORMAT_R8G8B8A8_UNORM;
    pipelineInfo.colorAttachmentFormats[3]      = FORMAT_R16G16B16A16_SFLOAT;                                               
    pipelineInfo.depthStencilAttachmentFormat   = EngineContext::Render()->GetDepthFormat();

    //if(!pipelineInfo.vertexShader->GetReflectInfo().DefinedSymbol("VERTEX_INPUT"));   // 也可以根据反射信息来检查管线适配性

    pipeline = GraphicsPipelineCache::Get()->Allocate(pipelineInfo).pipeline;     // 按原本提交的管线状态
    if(pipeline) return pipeline;                                                       // TODO 给定的着色器能否满足管线需要其实可以在材质绑定着色器的时候检查和设置标志位？
                                                                                        // 这样只用做一次而不是每帧检查
    pipelineInfo.vertexShader = pipelineState.clusterRender ? 
                                    pass->clusterVertexShader.shader : 
                                    pass->vertexShader.shader;                          // 用默认着色器
    pipelineInfo.geometryShader = nullptr;
    pipelineInfo.fragmentShader = pass->fragmentShader.shader;
    pipeline = GraphicsPipelineCache::Get()->Allocate(pipelineInfo).pipeline;
    if(pipeline) return pipeline;

    if(pipelineState.clusterRender) return pass->clusterPipeline;                       // 用默认管线
    else                            return pass->pipeline;                                                      
}

void GBufferPass::Init()
{
    meshPassProcessor = std::make_shared<GBufferPassProcessor>(this);
    MeshPass::Init();

    auto backend = EngineContext::RHI();

    vertexShader        = Shader(EngineContext::File()->ShaderPath() + "default/default.vert.spv", SHADER_FREQUENCY_VERTEX);
    clusterVertexShader = Shader(EngineContext::File()->ShaderPath() + "default/default_cluster.vert.spv", SHADER_FREQUENCY_VERTEX);
    fragmentShader      = Shader(EngineContext::File()->ShaderPath() + "default/deferred.frag.spv", SHADER_FREQUENCY_FRAGMENT);

    RHIRootSignatureInfo rootSignatureInfo = {};
    rootSignatureInfo.AddEntry(EngineContext::RenderResource()->GetPerFrameRootSignature()->GetInfo());                      
    rootSignature = backend->CreateRootSignature(rootSignatureInfo);

    RHIGraphicsPipelineInfo pipelineInfo = {};
    pipelineInfo.vertexShader       = vertexShader.shader;
    pipelineInfo.fragmentShader     = fragmentShader.shader;
    pipelineInfo.rootSignature      = rootSignature;
    pipelineInfo.primitiveType      = PRIMITIVE_TYPE_TRIANGLE_LIST;
    pipelineInfo.rasterizerState    = { FILL_MODE_SOLID, CULL_MODE_BACK, DEPTH_CLIP, 0.0f, 0.0f };
    for(uint32_t i = 0; i < 4; i++) pipelineInfo.blendState.renderTargets[i].enable = false;
    pipelineInfo.colorAttachmentFormats[0]      = FORMAT_R8G8B8A8_UNORM;
    pipelineInfo.colorAttachmentFormats[1]      = FORMAT_R8G8B8A8_SNORM;
    pipelineInfo.colorAttachmentFormats[2]      = FORMAT_R16G16B16A16_SFLOAT;
    pipelineInfo.colorAttachmentFormats[3]      = FORMAT_R32G32_SFLOAT;                                       
    pipelineInfo.depthStencilState              = { COMPARE_FUNCTION_LESS_EQUAL, true, true };
    pipelineInfo.depthStencilAttachmentFormat   = EngineContext::Render()->GetDepthFormat();
    pipeline                                    = GraphicsPipelineCache::Get()->Allocate(pipelineInfo).pipeline;    // 普通mesh的默认绘制管线

    pipelineInfo.vertexShader                   = clusterVertexShader.shader;
    clusterPipeline                             = GraphicsPipelineCache::Get()->Allocate(pipelineInfo).pipeline;    // cluster的默认绘制管线
}   

void GBufferPass::Build(RDGBuilder& builder) 
{
    Extent2D extent = EngineContext::Render()->GetWindowsExtent();

    RDGTextureHandle diffuse = builder.CreateTexture("G-Buffer Diffuse/Roughness")
        .Exetent({extent.width, extent.height, 1})
        .Format(FORMAT_R8G8B8A8_UNORM)
        .ArrayLayers(1)
        .MipLevels(1)
        .MemoryUsage(MEMORY_USAGE_GPU_ONLY)
        .AllowReadWrite()
        .AllowRenderTarget()
        .Finish();

    RDGTextureHandle normal = builder.CreateTexture("G-Buffer Normal/Metallic")
        .Exetent({extent.width, extent.height, 1})
        .Format(FORMAT_R8G8B8A8_SNORM)
        .ArrayLayers(1)
        .MipLevels(1)
        .MemoryUsage(MEMORY_USAGE_GPU_ONLY)
        .AllowReadWrite()
        .AllowRenderTarget()
        .Finish();

    RDGTextureHandle emission = builder.CreateTexture("G-Buffer Emission")
        .Exetent({extent.width, extent.height, 1})
        .Format(FORMAT_R16G16B16A16_SFLOAT)
        .ArrayLayers(1)
        .MipLevels(1)
        .MemoryUsage(MEMORY_USAGE_GPU_ONLY)
        .AllowReadWrite()
        .AllowRenderTarget()
        .Finish();

    RDGTextureHandle velocity = builder.CreateTexture("G-Buffer Velocity")
        .Import(EngineContext::RenderResource()->GetVelocityTexture(), RESOURCE_STATE_UNDEFINED)
        .Finish();

    // RDGTextureHandle pos = builder.CreateTexture("G-Buffer Position")
    //     .Exetent({extent.width, extent.height, 1})
    //     .Format(FORMAT_R16G16B16A16_SFLOAT)
    //     .ArrayLayers(1)
    //     .MipLevels(1)
    //     .MemoryUsage(MEMORY_USAGE_GPU_ONLY)
    //     .AllowReadWrite()
    //     .AllowRenderTarget()
    //     .Finish();

    // RDGTextureHandle depth = builder.CreateTexture("Depth")
    //     .Exetent({extent.width, extent.height, 1})
    //     .Format(EngineContext::Render()->GetDepthFormat())
    //     .ArrayLayers(1)
    //     .MipLevels(1)
    //     .MemoryUsage(MEMORY_USAGE_GPU_ONLY)
    //     .AllowDepthStencil()
    //     .Finish();  

    // RDGTextureHandle depth = builder.CreateTexture("Depth")
    //     .Import(EngineContext::RenderResource()->GetDepthTexture(), RESOURCE_STATE_UNDEFINED)
    //     .Finish();  

    RDGTextureHandle depth = builder.GetTexture("Depth");

    RDGRenderPassHandle pass = builder.CreateRenderPass(GetName())
        .Color(0, diffuse, ATTACHMENT_LOAD_OP_CLEAR, ATTACHMENT_STORE_OP_STORE, {0.0f, 0.0f, 0.0f, 0.0f})
        .Color(1, normal, ATTACHMENT_LOAD_OP_CLEAR, ATTACHMENT_STORE_OP_STORE, {0.0f, 0.0f, 0.0f, 0.0f})
        .Color(2, emission, ATTACHMENT_LOAD_OP_CLEAR, ATTACHMENT_STORE_OP_STORE, {0.0f, 0.0f, 0.0f, 0.0f})
        .Color(3, velocity, ATTACHMENT_LOAD_OP_CLEAR, ATTACHMENT_STORE_OP_STORE, {0.0f, 0.0f, 0.0f, 0.0f})
        .DepthStencil(depth, ATTACHMENT_LOAD_OP_LOAD, ATTACHMENT_STORE_OP_STORE, 1.0f, 0)
        .Execute([&](RDGPassContext context) {

            Extent2D windowExtent = EngineContext::Render()->GetWindowsExtent();

            RHICommandListRef command = context.command;  
            command->SetGraphicsPipeline(pipeline);                                          
            command->SetViewport({0, 0}, {windowExtent.width, windowExtent.height});
            command->SetScissor({0, 0}, {windowExtent.width, windowExtent.height}); 
            command->SetDepthBias(0.0f, 0.0f, 0.0f); 
            command->BindDescriptorSet(EngineContext::RenderResource()->GetPerFrameDescriptorSet(), 0);   

            meshPassProcessor->Draw(command);                      
        })
        .OutputRead(depth)  // 后续处理需要读深度
        .Finish();
}
