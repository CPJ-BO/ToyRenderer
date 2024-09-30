#include "ForwardPass.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RDG/RDGBuilder.h"
#include "Function/Render/RHI/RHIStructs.h"
#include "Function/Render/RenderResource/Material.h"
#include "MeshPass.h"
#include "RenderPass.h"
#include <memory>

void ForwardPassProcessor::OnCollectBatch(const DrawBatch& batch)
{
    if(batch.material->RenderPassMask() & PASS_MASK_FORWARD_PASS) AddBatch(batch);
}

RHIGraphicsPipelineRef ForwardPassProcessor::OnCreatePipeline(const DrawPipelineState& pipelineState)
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
    pipelineInfo.blendState.renderTargets[0].enable = false;
    pipelineInfo.colorAttachmentFormats[0]      = EngineContext::Render()->GetHdrColorFormat();                                            
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

void ForwardPass::Init()
{
    meshPassProcessor = std::make_shared<ForwardPassProcessor>(this);
    MeshPass::Init();

    auto backend = EngineContext::RHI();

    vertexShader        = Shader(EngineContext::File()->ShaderPath() + "default/default.vert.spv", SHADER_FREQUENCY_VERTEX);
    clusterVertexShader = Shader(EngineContext::File()->ShaderPath() + "default/default_cluster.vert.spv", SHADER_FREQUENCY_VERTEX);
    fragmentShader      = Shader(EngineContext::File()->ShaderPath() + "default/forward.frag.spv", SHADER_FREQUENCY_FRAGMENT);

    RHIRootSignatureInfo rootSignatureInfo = {};
    rootSignatureInfo.AddEntry(EngineContext::RenderResource()->GetPerFrameRootSignature()->GetInfo());                      
    rootSignature = backend->CreateRootSignature(rootSignatureInfo);

    RHIGraphicsPipelineInfo pipelineInfo = {};
    pipelineInfo.vertexShader       = vertexShader.shader;
    pipelineInfo.fragmentShader     = fragmentShader.shader;
    pipelineInfo.rootSignature      = rootSignature;
    pipelineInfo.primitiveType      = PRIMITIVE_TYPE_TRIANGLE_LIST;
    pipelineInfo.rasterizerState    = { FILL_MODE_SOLID, CULL_MODE_BACK, DEPTH_CLIP, 0.0f, 0.0f };
    pipelineInfo.blendState.renderTargets[0].enable = false;
    pipelineInfo.colorAttachmentFormats[0]      = EngineContext::Render()->GetHdrColorFormat();                                               
    pipelineInfo.depthStencilState              = { COMPARE_FUNCTION_LESS_EQUAL, true, true };
    pipelineInfo.depthStencilAttachmentFormat   = EngineContext::Render()->GetDepthFormat();
    pipeline                                    = GraphicsPipelineCache::Get()->Allocate(pipelineInfo).pipeline;    // 普通mesh的默认绘制管线

    pipelineInfo.vertexShader                   = clusterVertexShader.shader;
    clusterPipeline                             = GraphicsPipelineCache::Get()->Allocate(pipelineInfo).pipeline;    // cluster的默认绘制管线
}   

void ForwardPass::Build(RDGBuilder& builder) 
{
    RDGTextureHandle outColor = builder.GetTexture("Mesh Pass Out Color");

    RDGTextureHandle depth = builder.GetTexture("Depth");

    RDGRenderPassHandle pass = builder.CreateRenderPass(GetName())
        .Color(0, outColor, ATTACHMENT_LOAD_OP_LOAD, ATTACHMENT_STORE_OP_STORE, {0.0f, 0.0f, 0.0f, 0.0f})
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
